#include "defs.h"

// Static declares that this function can only be found in this file and not during linking
static void print_menu(int* choice);

/* Helper function declarations */
static void handle_load_sample(RoomCollection *rooms, EntryCollection *entries);
static void handle_print_entries(const EntryCollection *entries);
static void handle_print_rooms(const RoomCollection *rooms);
static void handle_add_room(RoomCollection *rooms);
static void handle_add_entry(RoomCollection *rooms, EntryCollection *entries);
static void handle_test_order(const EntryCollection *entries);
static void handle_test_rooms(const EntryCollection *entries, const RoomCollection *rooms);
static void read_room_name(char *room_name);
static int read_entry_data(int *timestamp, int *type, ReadingValue *value);

int main(void) {
    RoomCollection  rooms   = { .size = 0 };
    EntryCollection entries = { .size = 0 };

    // Stores user's menu selection
    int choice;
    
    // Main menu loop which runs forever until user chooses to exit
    while (1) {
             
        /* Display menu and get user's choice */
        print_menu(&choice);
        
        // User wants to exit
        if (choice == 0) {
            printf("Exiting program.\n");
            break;
        }
        else if (choice == 1) {
            // Pass addresses of both collections so they can be modified
            handle_load_sample(&rooms, &entries);
        }
        else if (choice == 2) {
            // Print all entries in sorted order
            handle_print_entries(&entries);
        }
        else if (choice == 3) {
            // Print all rooms with their entries
            handle_print_rooms(&rooms);
        }
        else if (choice == 4) {
            // Add a new room
            handle_add_room(&rooms);
        }
        else if (choice == 5) {
            // Add a new entry to an existing room
            handle_add_entry(&rooms, &entries);
        }
        else if (choice == 6) {
            // Test if entries are in correct sorted order
            handle_test_order(&entries);
        }
        else if (choice == 7) {
            // Test if room entry pointers are correct
            handle_test_rooms(&entries, &rooms);
        }
    }
    
    return 0;
}

void print_menu(int* choice) {
  int c = -1;
  int rc = 0;
  const int num_options = 7;

  printf("\nMAIN MENU\n");
  printf("  (1) Load sample data\n");
  printf("  (2) Print entries\n");
  printf("  (3) Print rooms\n");
  printf("  (4) Add room\n");
  printf("  (5) Add entry\n");
  printf("  (6) Test order\n");
  printf("  (7) Test room entries\n");
  printf("  (0) Exit\n\n");

  do {
    printf("Please enter a valid selection: ");
    // Check if they entered a non-integer
    rc = scanf("%d", &c);
    while (getchar() != '\n');
  } while (rc < 1 || c < 0 || c > num_options);

  *choice = c;
}

/* ---- handle_load_sample ---------------------------------------------------
   Purpose: Load pre-defined sample data into collections using loader.o.
   Params:
     - rooms (out): room collection to populate
     - entries (out): entry collection to populate
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void handle_load_sample(RoomCollection *rooms, EntryCollection *entries) {
    // Store the return code from load_sample
    int result;

    // Call the load_sample function from loader.o
    result = load_sample(rooms, entries);

    // Check if loading was successful 
    if (result == C_ERR_OK) {
        printf("Sample data loaded successfully.\n");
    }
    else {
        printf("Error loading sample data.\n");
    }
}

/* ---- handle_print_entries -------------------------------------------------
   Purpose: Print all entries in sorted order with column headers.
   Params:
     - entries (in): entry collection to print
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void handle_print_entries(const EntryCollection *entries) {
    // Loop counter
    int i;
    
    printf("\nAll Entries (sorted):\n");

    // Check if there are any entries to print
    if (entries->size > 0) {
        printf("%-15s %10s  %-10s  %s\n", "ROOM", "TIMESTAMP", "TYPE", "VALUE");
        printf("--------------- ----------  ----------  ---------------\n");

        // Loop through all entries and print each one
        for (i = 0; i < entries->size; i++) {
            // Get address of entry at position i and pass to entry_print
            entry_print(&entries->entries[i]);
        }
    }
    else {
        // No entries exist
        printf("  (No entries)\n");
    }
}

/* ---- handle_print_rooms ---------------------------------------------------
   Purpose: Print all rooms with their entries by calling room_print for each.
   Params:
     - rooms (in): room collection to print
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void handle_print_rooms(const RoomCollection *rooms) {
    // Loop counter
    int i;
    
    printf("\nAll Rooms:\n");

    // Check if there are any rooms to print 
    if (rooms->size > 0) {
        // Loop through all rooms 
        for (i = 0; i < rooms->size; i++) {
            // Print each room with its entries
            room_print(&rooms->rooms[i]);
        }
    }
    else {
        // No entries exist
        printf("  (No rooms)\n");
    }
}

/* ---- handle_add_room ------------------------------------------------------
   Purpose: Prompt user for room name and add it to the collection.
            Displays appropriate success or error messages.
   Params:
     - rooms (in/out): room collection to add to
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void handle_add_room(RoomCollection *rooms) {
    // Buffer to store the room name
    char room_name[MAX_STR];
    // Store return code from rooms_add
    int result;
    
    // Prompt user for a room name
    printf("Enter room name: ");
    read_room_name(room_name);
    
    result = rooms_add(rooms, room_name);
    
    // Check result and display appropriate message
    if (result == C_ERR_OK) {
        printf("Room '%s' added successfully.\n", room_name);
    }
    else if (result == C_ERR_DUPLICATE) {
        // Room already exists
        printf("Error: Room '%s' already exists.\n", room_name);
    }
    else if (result == C_ERR_FULL_ARRAY) {
        // Array is full
        printf("Error: Cannot add more rooms (maximum %d reached).\n", MAX_ARR);
    }
    else {
        printf("Error adding room.\n");
    }
}

/* ---- handle_add_entry -----------------------------------------------------
   Purpose: Prompt user for entry data (room, timestamp, type, value) and
            create a new entry in the specified room.
   Params:
     - rooms (in): room collection to find room in
     - entries (in/out): entry collection to add entry to
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void handle_add_entry(RoomCollection *rooms, EntryCollection *entries) {
    // Buffer to store room name 
    char room_name[MAX_STR];
    // Pointer to the room we'll add entry to
    Room *room;
    int timestamp;
    int type;
    // Union to store the actual reading value
    ReadingValue value;
    int result;
    
    // Get the room name 
    printf("Enter room name: ");
    read_room_name(room_name);
    
    // Try to find the room in our collection
    room = rooms_find(rooms, room_name);
    if (room == NULL) {
        // Room doesn't exist
        printf("Error: Room '%s' not found.\n", room_name);
        return;
    }
    
    // Read entry data from user
    if (read_entry_data(&timestamp, &type, &value) != C_ERR_OK) {
        // User entered invalid data
        printf("Error: Invalid entry data.\n");
        return;
    }
    
    // Create the entry
    result = entries_create(entries, room, type, value, timestamp);
    
    // Check result and display appropriate message 
    if (result == C_ERR_OK) {
        printf("Entry added successfully.\n");
    }
    else if (result == C_ERR_FULL_ARRAY) {
        // No more space in arrays
        printf("Error: Cannot add more entries (maximum reached).\n");
    }
    else if (result == C_ERR_INVALID) {
        // Invalid type or other validation error
        printf("Error: Invalid entry data.\n");
    }
    else {
        printf("Error adding entry.\n");
    }
}

/* ---- handle_test_order ----------------------------------------------------
   Purpose: Verify that entries are in correct sorted order using loader test.
   Params:
     - entries (in): entry collection to test
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void handle_test_order(const EntryCollection *entries) {
    // Store return code from test
    int result;
    
    // Call the test function from loader.o
    result = loader_test_order(entries, 1);

    // Display Result
    if (result == C_ERR_OK) {
        printf("Order test PASSED.\n");
    }
    else {
        printf("Order test FAILED.\n");
    }
}

/* ---- handle_test_rooms ----------------------------------------------------
   Purpose: Verify that room-entry linkages are correct and unique using
            loader test.
   Params:
     - entries (in): entry collection to test
     - rooms (in): room collection to test
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void handle_test_rooms(const EntryCollection *entries, const RoomCollection *rooms) {
    // Store return code from test
    int result;
    
    // Call the test function from loader.o
    // This verifies each entry appears exactly once in global array
    result = loader_test_rooms(entries, rooms, 1);

    // Display Result
    if (result == C_ERR_OK) {
        printf("Room entries test PASSED.\n");
    }
    else {
        printf("Room entries test FAILED.\n");
    }
}

/* ---- read_room_name -------------------------------------------------------
   Purpose: Read a room name from user input, supporting spaces in names.
            Uses scanf with [^\n] format to read entire line.
   Params:
     - room_name (out): buffer to store room name (must be at least MAX_STR)
   Returns: Nothing (void)
----------------------------------------------------------------------------- */
static void read_room_name(char *room_name) {
    // Reads up to 31 characters, stopping at newline
    scanf("%31[^\n]", room_name);

    // Removes the newline character
    while (getchar() != '\n');
}

/* ---- read_entry_data ------------------------------------------------------
   Purpose: Prompt user for and read entry data including timestamp, type,
            and type-specific value (temperature, decibels, or motion array).
   Params:
     - timestamp (out): pointer to store timestamp
     - type (out): pointer to store type (TYPE_TEMP, TYPE_DB, TYPE_MOTION)
     - value (out): pointer to union to store reading value
   Returns: C_ERR_OK if successful, C_ERR_INVALID if type is invalid
----------------------------------------------------------------------------- */
static int read_entry_data(int *timestamp, int *type, ReadingValue *value) {
    // Read timestamp
    printf("Enter timestamp: ");
    // Read integer into address pointed to by timestamp
    scanf("%d", timestamp);
    while (getchar() != '\n');
    
    // Read type 
    printf("Enter type (1=TEMP, 2=DB, 3=MOTION): ");
     // Read integer into address pointed to by type
    scanf("%d", type);
    while (getchar() != '\n');
    
    // Based on type, reads appropriate value 
    if (*type == TYPE_TEMP) {
        printf("Enter temperature (float): ");
        // &value->temperature: address of temperature field in the union 
        scanf("%f", &value->temperature);
        while (getchar() != '\n');
    }
    else if (*type == TYPE_DB) {
        printf("Enter decibels (int): ");
        scanf("%d", &value->decibels);
        while (getchar() != '\n');
    }
    else if (*type == TYPE_MOTION) {
        // Reads 3 unsigned char values
        printf("Enter motion values (3 integers 0 or 1): ");
        scanf("%hhu %hhu %hhu", &value->motion[0], &value->motion[1], &value->motion[2]);
        while (getchar() != '\n');
    }
    else {
        return C_ERR_INVALID;
    }
    
    return C_ERR_OK;
}
