#include "defs.h"

// Helper function declarations
static int find_insertion_position(const EntryCollection *ec, const LogEntry *new_entry);
static void retarget_room_pointer(LogEntry *dst, LogEntry *src);
static void shift_entries_right(EntryCollection *ec, int insert_pos);
static void insert_pointer_in_room(Room *room, LogEntry *entry);

/* ---- entry comparator -------------------------------------------
   Order: room name ASC, then type ASC by #define value, then timestamp ASC
   Returns <0 if a<b, >0 if a>b, 0 if equal.
----------------------------------------------------------------------------- */
int entry_cmp(const LogEntry *a, const LogEntry *b) {
    // Store result of room name comparison
    int room_cmp; 
      
    // Checks for empty pointers to prevent crashes
    if (a == NULL || b == NULL) {
        return 0;
    }
    
    // Check for empty room pointers to make sure the entries have valid rooms
    if (a->room == NULL || b->room == NULL) {
        return 0;
    }
    
    /* First, compare room names in an ascending alphabetically order by using
       strncmp compares strings and returns negative if a < b, positive if a > b, 0 if equal */
    room_cmp = strncmp(a->room->name, b->room->name, MAX_STR);
    
    if (room_cmp != 0) {
        // If rooms are different, then return the comparison result
        return room_cmp;
    }
    
    // If rooms are the same, then compare types in an ascending by numeric value
    if (a->data.type < b->data.type) {
        return -1;
    }
    if (a->data.type > b->data.type) {
        return 1;
    }
    
    // If the types are the same, then compare timestamps in a ascending order
    if (a->timestamp < b->timestamp) {
        return -1;
    }
    if (a->timestamp > b->timestamp) {
        return 1;
    }
    
    // If all fields are equal
    return 0;
    
}

/* ---- rooms_find ------------------------------------------------------------
   Purpose: Find a room by name.
   Params:
    - rc (in): room collection
    - room_name (in): C-string room name
   Returns: pointer to room or NULL if not found or on error
----------------------------------------------------------------------------- */
Room* rooms_find(RoomCollection *rc, const char *room_name) {
    // Loop counter for iterating through rooms
    int i;
      
    // Checks for empty pointers to prevent crashes
    if (rc == NULL || room_name == NULL) {
        return NULL;
    }
    
    // Search through all rooms in the collection
    // Start at index 0, go until we reach the current size
    for (i = 0; i < rc->size; i++) {
        // Compare current room's name with the name we're searching for
        if (strncmp(rc->rooms[i].name, room_name, MAX_STR) == 0) {
            // If found, then return a pointer to this room
            return &rc->rooms[i];
        }
    }
    
    // Room not found
    return NULL;
    
}

/* ---- rooms_add -------------------------------------------------------------
   Purpose: Add a room if it does not already exist.
   Params:
     - rc (in/out): room collection
     - room_name (in): C-string room name
   Returns: C_ERR_OK, C_ERR_NULL_PTR, C_ERR_DUPLICATE, C_ERR_FULL_ARRAY
----------------------------------------------------------------------------- */
int rooms_add(RoomCollection *rc, const char *room_name) {
    // The pointer to the new room that we'll create
    Room *new_room;
      
    // Checks for empty pointers to prevent crashes
    if (rc == NULL || room_name == NULL) {
        return C_ERR_NULL_PTR;
    }
    
    // Check if array is full
    if (rc->size >= MAX_ARR) {
        return C_ERR_FULL_ARRAY;
    }
    
    // Check if room already exists
    if (rooms_find(rc, room_name) != NULL) {
        return C_ERR_DUPLICATE;
    }
    
    // Add the new room at the end
    new_room = &rc->rooms[rc->size];
    
    // Copy the room name uing stncpy 
    strncpy(new_room->name, room_name, MAX_STR - 1);
    // Add null terminator at the end to ensure string is valid
    new_room->name[MAX_STR - 1] = '\0';
    
    // Initialize the room's entry collection
    new_room->size = 0;
    
    // Increment the collection size
    rc->size++;
    
    return C_ERR_OK;

}

/* ---- find_insertion_position ----------------------------------------------
   Purpose: Find the correct sorted position for a new entry in the collection.
   Params:
     - ec (in): entry collection to search
     - new_entry (in): entry to find position for
   Returns: Index where new_entry should be inserted to maintain sorted order
----------------------------------------------------------------------------- */
static int find_insertion_position(const EntryCollection *ec, const LogEntry *new_entry) {
    // Loop counter
    int i;
    
    // Loop through all existing entries
    for (i = 0; i < ec->size; i++) {
        // Compare new_entry with entry at position i
        // If new_entry comes before current entry, then it's a negative result
        if (entry_cmp(new_entry, &ec->entries[i]) < 0) {
            return i;
        }
    }

    // Return the position of the current size
    return ec->size;

}

/* ---- retarget_room_pointer ------------------------------------------------
   Purpose: Update a room's pointer after an entry has been shifted in memory.
            After copying an entry from src to dst during array shift, this
            function finds and updates the pointer in the room's entries array.
   Params:
     - dst (in): destination entry (new location after shift)
     - src (in): source entry (old location before shift)
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void retarget_room_pointer(LogEntry *dst, LogEntry *src) {
    // Loop counter
    int j;
    
    // Go through all pointers in the room's entry array
    // We need to find which pointer was pointing to the old location
    for (j = 0; j < dst->room->size; j++) {
        // Check if this pointer points to the old location
        if (dst->room->entries[j] == src) {
            // Update it to point to the new location
            dst->room->entries[j] = dst;
            break;
        }
    }

}

/* ---- shift_entries_right --------------------------------------------------
   Purpose: Shift all entries from insert_pos to the end one position right,
            retargeting room pointers for each shifted entry to maintain
            pointer validity.
   Params:
     - ec (in/out): entry collection to shift
     - insert_pos (in): position where new entry will be inserted
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void shift_entries_right(EntryCollection *ec, int insert_pos) {
    // Loop counter
    int i;
    
    // Start from the end and work backwards to avoid overwriting data
    for (i = ec->size; i > insert_pos; i--) {
        // Copy entry from position i-1 to position i
        ec->entries[i] = ec->entries[i - 1];

        // The entry moved from &ec->entries[i-1] to &ec->entries[i]
        // Any room that was pointing to the old address must now point to new address
        retarget_room_pointer(&ec->entries[i], &ec->entries[i - 1]);
    }

}

/* ---- insert_pointer_in_room -----------------------------------------------
   Purpose: Insert a pointer to an entry into a room's sorted pointer array.
            Finds the correct sorted position and shifts existing pointers
            to make space.
   Params:
     - room (in/out): room to insert pointer into
     - entry (in): pointer to entry to insert
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void insert_pointer_in_room(Room *room, LogEntry *entry) {
    // The position where pointer should be inserted
    int insert_pos;
    // Loop counter
    int i;
    
    // Find insertion position
    insert_pos = room->size;

    // Search through existing pointers to find correct sorted position
    for (i = 0; i < room->size; i++) {

        // If our entry comes before the entry at position i
        if (entry_cmp(entry, room->entries[i]) < 0) {
            insert_pos = i;
            break;
        }
    }
    
    // Shift pointers right to make space
    for (i = room->size; i > insert_pos; i--) {
        room->entries[i] = room->entries[i - 1];
    }
    
    // Insert new pointer at the correct position
    room->entries[insert_pos] = entry;

    // Increment room's entry count
    room->size++;
}

/* ---- entries_create -----------------------------------------------------------
   Purpose: Create a log entry and place it in the global entries (sorted),
            and attach a pointer to it in the owning room (append or sorted, either works)
   Params:
     - ec (in/out): entry collection (owns LogEntry storage)
     - room (in/out): room to attach entry to (must already exist)
     - type (in): TYPE_TEMP|TYPE_DB|TYPE_MOTION
     - value (in): union payload for reading
     - timestamp (in): simple int timestamp
   Returns: C_ERR_OK, C_ERR_NULL_PTR, C_ERR_FULL_ARRAY, C_ERR_INVALID
----------------------------------------------------------------------------- */
int entries_create(EntryCollection *ec, Room *room, int type, ReadingValue value, int timestamp) {
    // The new entry we'll create
    LogEntry new_entry;
    // Where to insert it in sorted order
    int insert_pos;
    
    // Check for empty pointers
    if (ec == NULL || room == NULL) {
        return C_ERR_NULL_PTR;
    }
    
    // Validate type
    if (type != TYPE_TEMP && type != TYPE_DB && type != TYPE_MOTION) {
        return C_ERR_INVALID;
    }
    
    // Check capacity
    if (ec->size >= MAX_ARR || room->size >= MAX_ARR) {
        return C_ERR_FULL_ARRAY;
    }
    
    // Create the new entry
    new_entry.data.type = type;
    new_entry.data.value = value;
    new_entry.room = room;
    new_entry.timestamp = timestamp;
    
    // Find where to insert in sorted order
    insert_pos = find_insertion_position(ec, &new_entry);

    // Shift existing entries to make space and retarget pointers
    shift_entries_right(ec, insert_pos);
    
    // Insert the new entry at the correct position
    ec->entries[insert_pos] = new_entry;
    ec->size++;
    
    // Insert pointer in room's array
    // Pass the address of the entry we just inserted
    insert_pointer_in_room(room, &ec->entries[insert_pos]);
    
    return C_ERR_OK;    
}

/* ---- entry_print -----------------------------------------------------------
   Purpose: Print one entry in a formatted row.
   Params:
     - e (in): entry to print
   Returns: C_ERR_OK, C_ERR_NULL_PTR if e is NULL, C_ERR_INVALID if room is NULL
----------------------------------------------------------------------------- */
int entry_print(const LogEntry *e) {
    // Loop counter
    int i;
    
    // Check for empty entry
    if (e == NULL) {
        return C_ERR_NULL_PTR;
    }
    
    // Check for emoty room
    if (e->room == NULL) {
        return C_ERR_INVALID;
    }
    
    // Print room name and timestamp
    printf("%-15s %10d  ", e->room->name, e->timestamp);
    
    // Print type and value based on type
    if (e->data.type == TYPE_TEMP) {
        printf("%-10s  %.2f°C\n", "TEMP", e->data.value.temperature);
    }
    else if (e->data.type == TYPE_DB) {
        printf("%-10s  %d dB\n", "DB", e->data.value.decibels);
    }
    else if (e->data.type == TYPE_MOTION) {
        printf("%-10s  [", "MOTION");

        // Loop through the 3-element motion array
        for (i = 0; i < 3; i++) {
            printf("%d", e->data.value.motion[i]);
            if (i < 2) {
                printf(",");
            }
        }
        printf("]\n");
    }
    else {
        // Unknown type
        return C_ERR_INVALID;
    }
    
    return C_ERR_OK;
}

/* ---- room_print ------------------------------------------------------------
   Purpose: Print a room header and all of its entries (already sorted).
   Params:
     - r (in): room to print
   Returns: C_ERR_OK, C_ERR_NULL_PTR if r is NULL
----------------------------------------------------------------------------- */
int room_print(const Room *r) {
    // Loop counter
    int i;
    // Store result from entry_print
    int result;
    
    // Check for empty room
    if (r == NULL) {
        return C_ERR_NULL_PTR;
    }
    
    // Print room header with name and entry count
    printf("\nRoom: %s (entries=%d)\n", r->name, r->size);
    
    // Print column headers if there are entries to display
    if (r->size > 0) {
        printf("%-15s %10s  %-10s  %s\n", "ROOM", "TIMESTAMP", "TYPE", "VALUE");
        printf("--------------- ----------  ----------  ---------------\n");
        
        // Print each entry in the room
        for (i = 0; i < r->size; i++) {

            // Call entry_print for each pointer in room's entries array
            result = entry_print(r->entries[i]);

            // Check if printing failed
            if (result != C_ERR_OK) {
                printf("Error printing entry %d\n", i);
            }
        }
    }
    else {
        printf("  (No entries)\n");
    }
    
    return C_ERR_OK;
}