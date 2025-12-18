#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <string.h>

#define MAX_ARR   16
#define MAX_STR   32

#define C_ERR_OK          0
#define C_ERR_NULL_PTR   -1
#define C_ERR_FULL_ARRAY -2
#define C_ERR_NOT_FOUND  -3
#define C_ERR_DUPLICATE  -4
#define C_ERR_INVALID    -5
#define C_ERR_NOT_IMPLEMENTED -99 // No function should return this by the end of your assignment

/* NOTE: Enumerated Data Types might be better for this, but we have not discussed these. */
#define TYPE_TEMP    1
#define TYPE_DB      2
#define TYPE_MOTION  3

typedef struct Room     Room;
typedef struct LogEntry LogEntry;

typedef union {
    float         temperature;   /* °C */
    int           decibels;      /* dB */
    unsigned char motion[3];     /* {left, forward, right} as either 0 or 1 */
} ReadingValue;

typedef struct {
    int          type;           /* TYPE_* defined valule */
    ReadingValue value;
} Reading;

/* One log entry belongs to a room and has a timestamp */
struct LogEntry {
    Reading  data;
    Room    *room;
    int      timestamp;
};

/* One room has a name and a collection of pointers to its log entries */
struct Room {
    char      name[MAX_STR];
    LogEntry* entries[MAX_ARR];
    int       size;
};

typedef struct {
    Room rooms[MAX_ARR];
    int  size;
} RoomCollection;

typedef struct {
    LogEntry entries[MAX_ARR];
    int      size;
} EntryCollection;


int rooms_add(RoomCollection *rc, const char *room_name);
int entries_create(EntryCollection *ec,
                Room            *room,
                int              type,
                ReadingValue     value,
                int              timestamp);

Room* rooms_find(RoomCollection *rc, const char *room_name);
int room_print(const Room *r);
int entry_print(const LogEntry *e);
int entry_cmp(const LogEntry *a, const LogEntry *b);


/* =========================================
   Loader (provided as an object file)
   =========================================
   load_sample: Override the contents of the collections with sample data.
    - rc (out): room collection
    - ec (out): entry collection
    - Returns: C_ERR_OK, C_ERR_NULL_PTR

   loader_test_order: verify that the entries are in correct order.
    - verbose (in): if non-zero, print out the entries as we check them
    - Returns: C_ERR_OK, C_ERR_NULL_PTR, C_ERR_INVALID (invalid means the entries are not in sorted order)

    loader_test_rooms: verify that the rooms and their entries are consistent and unique.
    - ec (in): entry collection
    - rc (in): room collection
    - verbose (in): if non-zero, print out errors as we find them
    - Returns: C_ERR_OK, C_ERR_NULL_PTR, C_ERR_INVALID (invalid means there is a problem with the room/entry linkage)
   ========================================= */
int load_sample(RoomCollection *rc, EntryCollection *ec);
int loader_test_order(const EntryCollection *ec, int verbose);
int loader_test_rooms(const EntryCollection *ec, const RoomCollection *rc, int verbose);

#endif /* DEFS_H */
