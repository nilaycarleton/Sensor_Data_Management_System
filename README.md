# Sensor Data Management System

A C-based sensor monitoring system that tracks readings from temperature, decibel, and motion sensors across multiple rooms.

## Overview

This program implements a data management system for IoT sensor readings in a building. It maintains a collection of rooms and their associated sensor log entries, automatically sorting entries by room name, sensor type, and timestamp. The system demonstrates advanced data structure management with sorted insertion and pointer-based relationships.

### Key Features

- **Multi-Room Tracking**: Manage multiple rooms with unique names
- **Three Sensor Types**: Temperature (°C), Decibels (dB), and Motion detection
- **Automatic Sorting**: Entries sorted by room → type → timestamp
- **Dual Collections**: Global sorted entries + per-room entry pointers
- **Data Validation**: Built-in testing for sort order and pointer consistency
- **Interactive Menu**: User-friendly command-line interface
- **Sample Data Loading**: Pre-configured test data for validation

## Project Structure

```
.
├── main.c              # Program entry point and menu handlers
├── manager.c           # Core data management functions
├── defs.h              # Type definitions and constants
├── loader.o            # Precompiled sample data loader (provided)
└── README.md           # This file
```

## Data Structures

### Reading Types
```c
#define TYPE_TEMP    1  // Temperature sensor (float °C)
#define TYPE_DB      2  // Decibel meter (int dB)
#define TYPE_MOTION  3  // Motion detector (3-element array)
```

### ReadingValue Union
```c
typedef union {
    float         temperature;   // °C (TYPE_TEMP)
    int           decibels;      // dB (TYPE_DB)
    unsigned char motion[3];     // [left, forward, right] (TYPE_MOTION)
} ReadingValue;
```

**Union Advantage**: Stores only one type at a time, saving memory.

### Reading Structure
```c
typedef struct {
    int          type;           // TYPE_TEMP, TYPE_DB, or TYPE_MOTION
    ReadingValue value;          // Union containing the actual reading
} Reading;
```

### LogEntry Structure
```c
struct LogEntry {
    Reading  data;               // The sensor reading
    Room    *room;              // Pointer to owning room
    int      timestamp;         // Simple integer timestamp
};
```

### Room Structure
```c
struct Room {
    char      name[MAX_STR];    // Room name (max 32 chars)
    LogEntry* entries[MAX_ARR]; // Pointers to this room's entries
    int       size;             // Number of entries in this room
};
```

**Key Design**: Room doesn't own the entry data, only pointers to entries in the global collection.

### Collections
```c
typedef struct {
    Room rooms[MAX_ARR];        // Fixed array of rooms (max 16)
    int  size;                  // Current number of rooms
} RoomCollection;

typedef struct {
    LogEntry entries[MAX_ARR];  // Fixed array of entries (max 16)
    int      size;              // Current number of entries
} EntryCollection;
```

## Requirements

- **Compiler**: GCC with C standard library
- **System**: Linux/Unix environment
- **Provided Files**: `loader.o` (precompiled sample data loader)

## Installation & Building

### Extract Archive
```bash
tar -xvf a2_101345637.tar
cd assignment2
```

### Compilation
```bash
gcc -Wall main.c manager.c loader.o -o a2
```

**Compiler Flags**:
- `-Wall`: Enable all warnings
- `-o a2`: Output executable name

### Verify Compilation
```bash
ls -l a2
```

## Usage

### Running the Program
```bash
./a2
```

### Main Menu

```
MAIN MENU
  (1) Load sample data
  (2) Print entries
  (3) Print rooms
  (4) Add room
  (5) Add entry
  (6) Test order
  (7) Test room entries
  (0) Exit

Please enter a valid selection:
```

### Menu Options Explained

#### 1. Load Sample Data
Loads pre-configured test data from `loader.o`:
- Multiple rooms (Kitchen, Living Room, Bedroom, etc.)
- Various sensor readings with different timestamps
- Tests sorting and pointer management

**Output**:
```
Sample data loaded successfully.
```

---

#### 2. Print Entries
Displays all entries in sorted order:
1. **Primary sort**: Room name (alphabetical)
2. **Secondary sort**: Type (TEMP=1, DB=2, MOTION=3)
3. **Tertiary sort**: Timestamp (ascending)

**Sample Output**:
```
All Entries (sorted):
ROOM            TIMESTAMP  TYPE        VALUE
--------------- ----------  ----------  ---------------
Bedroom              100  TEMP        22.50°C
Bedroom              150  TEMP        23.00°C
Bedroom              200  DB          45 dB
Kitchen              100  TEMP        24.00°C
Kitchen              100  MOTION      [1,0,1]
Living Room          50   TEMP        21.00°C
```

---

#### 3. Print Rooms
Displays each room with its associated entries:

**Sample Output**:
```
All Rooms:

Room: Bedroom (entries=3)
ROOM            TIMESTAMP  TYPE        VALUE
--------------- ----------  ----------  ---------------
Bedroom              100  TEMP        22.50°C
Bedroom              150  TEMP        23.00°C
Bedroom              200  DB          45 dB

Room: Kitchen (entries=2)
ROOM            TIMESTAMP  TYPE        VALUE
--------------- ----------  ----------  ---------------
Kitchen              100  TEMP        24.00°C
Kitchen              100  MOTION      [1,0,1]
```

---

#### 4. Add Room
Creates a new room with a unique name.

**Interaction**:
```
Please enter a valid selection: 4
Enter room name: Library
Room 'Library' added successfully.
```

**Error Cases**:
```
Error: Room 'Library' already exists.
Error: Cannot add more rooms (maximum 16 reached).
```

---

#### 5. Add Entry
Creates a sensor reading for an existing room.

**Temperature Entry**:
```
Please enter a valid selection: 5
Enter room name: Library
Enter timestamp: 150
Enter type (1=TEMP, 2=DB, 3=MOTION): 1
Enter temperature (float): 22.5
Entry added successfully.
```

**Decibel Entry**:
```
Enter room name: Kitchen
Enter timestamp: 200
Enter type (1=TEMP, 2=DB, 3=MOTION): 2
Enter decibels (int): 55
Entry added successfully.
```

**Motion Entry**:
```
Enter room name: Hallway
Enter timestamp: 300
Enter type (1=TEMP, 2=DB, 3=MOTION): 3
Enter motion values (3 integers 0 or 1): 1 1 0
Entry added successfully.
```

**Motion Array**: `[left, forward, right]` each 0 (no motion) or 1 (motion detected)

---

#### 6. Test Order
Validates that entries are correctly sorted using the provided test function.

**Output**:
```
Order test PASSED.
```

Or if sorting is incorrect:
```
Order test FAILED.
```

---

#### 7. Test Room Entries
Verifies that:
- Each entry appears exactly once in the global array
- Room pointers correctly reference entries
- No duplicate or missing pointers

**Output**:
```
Room entries test PASSED.
```

---

#### 0. Exit
Cleanly exits the program.

```
Exiting program.
```

## Core Functions

### `entry_cmp()`
```c
int entry_cmp(const LogEntry *a, const LogEntry *b);
```

**Purpose**: Compares two entries for sorting.

**Sort Order**:
1. **Room name** (alphabetical, case-sensitive)
2. **Type** (TYPE_TEMP < TYPE_DB < TYPE_MOTION)
3. **Timestamp** (ascending)

**Returns**:
- Negative: a < b
- Zero: a == b
- Positive: a > b

**Implementation**:
```c
// Compare room names
int room_cmp = strncmp(a->room->name, b->room->name, MAX_STR);
if (room_cmp != 0) return room_cmp;

// Compare types
if (a->data.type < b->data.type) return -1;
if (a->data.type > b->data.type) return 1;

// Compare timestamps
if (a->timestamp < b->timestamp) return -1;
if (a->timestamp > b->timestamp) return 1;

return 0;  // Equal
```

---

### `rooms_find()`
```c
Room* rooms_find(RoomCollection *rc, const char *room_name);
```

**Purpose**: Searches for a room by name.

**Returns**:
- Pointer to room if found
- `NULL` if not found or on error

**Algorithm**: Linear search with `strncmp()`

---

### `rooms_add()`
```c
int rooms_add(RoomCollection *rc, const char *room_name);
```

**Purpose**: Adds a new room if it doesn't already exist.

**Validation**:
- Checks for NULL pointers
- Verifies array capacity (max 16)
- Prevents duplicate names

**Returns**:
- `C_ERR_OK`: Success
- `C_ERR_NULL_PTR`: Invalid pointer
- `C_ERR_DUPLICATE`: Room already exists
- `C_ERR_FULL_ARRAY`: Maximum capacity reached

---

### `entries_create()`
```c
int entries_create(EntryCollection *ec, Room *room, int type, 
                   ReadingValue value, int timestamp);
```

**Purpose**: Creates a new sensor reading entry with sorted insertion.

**Algorithm**:
1. Validate inputs (pointers, type, capacity)
2. Create new LogEntry structure
3. Find insertion position using `entry_cmp()`
4. Shift existing entries right to make space
5. Update room pointers that were shifted
6. Insert new entry at correct position
7. Add pointer to room's entry array

**Critical Operations**:
- **Sorted Insertion**: Maintains global sort order
- **Pointer Retargeting**: Updates room pointers after shifts
- **Dual Collection Update**: Updates both global and room-specific arrays

**Returns**:
- `C_ERR_OK`: Success
- `C_ERR_NULL_PTR`: Invalid pointer
- `C_ERR_INVALID`: Invalid type
- `C_ERR_FULL_ARRAY`: Array capacity exceeded

---

### `find_insertion_position()`
```c
static int find_insertion_position(const EntryCollection *ec, 
                                   const LogEntry *new_entry);
```

**Purpose**: Finds where to insert new entry to maintain sort order.

**Algorithm**: Linear search using `entry_cmp()`

**Returns**: Index where entry should be inserted

---

### `shift_entries_right()`
```c
static void shift_entries_right(EntryCollection *ec, int insert_pos);
```

**Purpose**: Shifts all entries from insert_pos to end one position right.

**Critical Feature**: Calls `retarget_room_pointer()` for each shifted entry.

**Process**:
```
Before shift:
entries[0] [1] [2] [3] [4]
               ↑
           insert here

After shift:
entries[0] [1] [__] [2] [3] [4]
               ↑
           ready for insertion
```

---

### `retarget_room_pointer()`
```c
static void retarget_room_pointer(LogEntry *dst, LogEntry *src);
```

**Purpose**: Updates room pointer after entry moves in memory.

**Why Needed**: When entries shift, their addresses change, but room pointers must still point to correct entries.

**Algorithm**:
1. Search room's entry pointer array
2. Find pointer matching old address (src)
3. Update it to new address (dst)

---

### `insert_pointer_in_room()`
```c
static void insert_pointer_in_room(Room *room, LogEntry *entry);
```

**Purpose**: Adds entry pointer to room's array in sorted order.

**Note**: Room's array maintains same sort order as global array.

---

### `entry_print()`
```c
int entry_print(const LogEntry *e);
```

**Purpose**: Prints formatted entry with type-specific value display.

**Output Format**:
```
ROOM            TIMESTAMP  TYPE        VALUE
--------------- ----------  ----------  ---------------
Kitchen              150  TEMP        24.00°C
Bedroom              200  DB          55 dB
Hallway              300  MOTION      [1,1,0]
```

---

### `room_print()`
```c
int room_print(const Room *r);
```

**Purpose**: Prints room header and all its entries.

**Algorithm**: Iterates through room's entry pointer array calling `entry_print()`.

## Error Codes

| Code | Constant | Meaning |
|------|----------|---------|
| 0 | `C_ERR_OK` | Success |
| -1 | `C_ERR_NULL_PTR` | NULL pointer parameter |
| -2 | `C_ERR_FULL_ARRAY` | Array at maximum capacity |
| -3 | `C_ERR_NOT_FOUND` | Item not found |
| -4 | `C_ERR_DUPLICATE` | Duplicate item |
| -5 | `C_ERR_INVALID` | Invalid parameter value |
| -99 | `C_ERR_NOT_IMPLEMENTED` | Feature not yet implemented |

## Pointer Architecture

### Two-Level Structure

**Global Collection** (EntryCollection):
- Owns the actual LogEntry data
- Maintains sorted order
- Fixed array of 16 entries max

**Room Collections** (Room.entries):
- Stores pointers to entries in global collection
- Does NOT own the data
- Each room maintains pointers to its entries
- Also maintains sorted order

### Why This Design?

**Advantages**:
1. Single source of truth (global array)
2. Efficient room-specific queries
3. Automatic consistency (pointers reference same data)
4. Memory efficient (one copy of data)

**Challenges**:
- Must update pointers when entries shift
- More complex insertion logic
- Pointer consistency critical

### Pointer Retargeting Example

```
Before insertion:
Global: [A] [B] [D] [__]
Room->entries: [&A, &B, &D]

Insert C between B and D:
Step 1 - Shift D right:
Global: [A] [B] [__] [D']
        D moved from &entries[2] to &entries[3]

Step 2 - Retarget room pointer:
Room->entries: [&A, &B, &entries[3]]
                          ↑
                    Was &entries[2], now &entries[3]

Step 3 - Insert C:
Global: [A] [B] [C] [D']
Room->entries: [&A, &B, &C, &entries[3]]
```

## Algorithm Complexity

### Time Complexity

| Function | Worst Case | Notes |
|----------|-----------|-------|
| `rooms_find()` | O(n) | Linear search, n = number of rooms |
| `rooms_add()` | O(n) | Includes duplicate check |
| `entries_create()` | O(n²) | Find position O(n), shift O(n), retarget O(n²) |
| `entry_cmp()` | O(1) | Constant time comparison |
| `entry_print()` | O(1) | Fixed output |
| `room_print()` | O(m) | m = entries in room |

### Space Complexity

| Structure | Space | Max Size |
|-----------|-------|----------|
| Room | ~300 bytes | 32-char name + 16 pointers + metadata |
| LogEntry | ~50 bytes | Reading + pointer + timestamp |
| RoomCollection | ~4.8 KB | 16 rooms max |
| EntryCollection | ~800 bytes | 16 entries max |

**Total**: ~5.6 KB for maximum capacity

## Sample Usage Session

```bash
$ ./a2

MAIN MENU
  (1) Load sample data
  (2) Print entries
  (3) Print rooms
  (4) Add room
  (5) Add entry
  (6) Test order
  (7) Test room entries
  (0) Exit

Please enter a valid selection: 4
Enter room name: Server Room
Room 'Server Room' added successfully.

Please enter a valid selection: 5
Enter room name: Server Room
Enter timestamp: 1000
Enter type (1=TEMP, 2=DB, 3=MOTION): 1
Enter temperature (float): 18.5
Entry added successfully.

Please enter a valid selection: 5
Enter room name: Server Room
Enter timestamp: 1050
Enter type (1=TEMP, 2=DB, 3=MOTION): 2
Enter decibels (int): 42
Entry added successfully.

Please enter a valid selection: 3

All Rooms:

Room: Server Room (entries=2)
ROOM            TIMESTAMP  TYPE        VALUE
--------------- ----------  ----------  ---------------
Server Room         1000  TEMP        18.50°C
Server Room         1050  DB          42 dB

Please enter a valid selection: 6
Order test PASSED.

Please enter a valid selection: 7
Room entries test PASSED.

Please enter a valid selection: 0
Exiting program.
```

## Testing & Validation

### Built-in Tests

**Option 6: Test Order**
- Validates entries are in correct sorted order
- Uses `loader_test_order()` from loader.o
- Compares each consecutive pair with `entry_cmp()`

**Option 7: Test Room Entries**
- Verifies each entry appears exactly once
- Checks room pointers are valid
- Uses `loader_test_rooms()` from loader.o

### Manual Testing

**Test Case 1: Sorted Insertion**
```
Add rooms: Alpha, Zeta, Beta
Add entries with varying timestamps
Verify print shows: Alpha entries, Beta entries, Zeta entries
```

**Test Case 2: Type Ordering**
```
Add entries to same room:
- Timestamp 100, TYPE_MOTION
- Timestamp 100, TYPE_TEMP
- Timestamp 100, TYPE_DB

Expected order: TEMP, DB, MOTION
```

**Test Case 3: Pointer Consistency**
```
Add 10 entries to 3 different rooms
Run Option 7 (Test room entries)
Should show: PASSED
```

**Test Case 4: Capacity Limits**
```
Add 16 rooms (should succeed)
Try adding 17th room (should fail: FULL_ARRAY)
```

## Common Issues & Solutions

### Compilation Errors

**Problem**: `undefined reference to 'load_sample'`
**Solution**: Include loader.o in compilation:
```bash
gcc -Wall main.c manager.c loader.o -o a2
```

### Runtime Issues

**Problem**: Segmentation fault when printing entries
**Solution**: Ensure entries have valid room pointers before printing

**Problem**: Test order fails
**Debug**:
1. Print entries manually
2. Check room name comparison (case-sensitive)
3. Verify type values (1, 2, 3)
4. Check timestamp ordering

**Problem**: Test room entries fails
**Debug**:
1. Verify `retarget_room_pointer()` is called after shifts
2. Check pointer updates in `shift_entries_right()`
3. Ensure room size incremented in `insert_pointer_in_room()`

### Logic Errors

**Problem**: Entries not properly sorted
**Solution**: Verify `entry_cmp()` implementation matches specification

**Problem**: Duplicate entries in room
**Solution**: Check `insert_pointer_in_room()` doesn't double-add

## Constants Reference

```c
#define MAX_ARR 16         // Maximum rooms/entries
#define MAX_STR 32         // Maximum string length

#define TYPE_TEMP 1        // Temperature sensor
#define TYPE_DB 2          // Decibel meter  
#define TYPE_MOTION 3      // Motion detector
```

## Learning Objectives Demonstrated

- ✅ Complex data structure design
- ✅ Pointer-based relationships
- ✅ Union types for memory efficiency
- ✅ Sorted insertion algorithms
- ✅ Array manipulation and shifting
- ✅ Pointer retargeting after memory moves
- ✅ Modular function design
- ✅ Error handling and validation
- ✅ Working with precompiled libraries (.o files)
- ✅ Maintaining data consistency across structures
- ✅ Interactive menu systems
- ✅ Formatted output with printf

## License

This project was created for academic purposes at Carleton University. All rights reserved.
