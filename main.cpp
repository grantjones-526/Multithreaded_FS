#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "FileSystem.h"
#include "ptOFT.h"

// ============================================================
// Helper: open a file and register it in a thread's open file table
// ============================================================
ThreadFileEntry* open_and_register(FileSystem& fs, ThreadOpenFileTable& table, string file_name) {
    ThreadFileEntry* entry = fs.open(file_name);
    if (entry == nullptr) return nullptr;
    // Add to the thread's table using the system-wide entry from open()
    ThreadFileEntry* registered = table.add_entry(file_name, entry->system_entry);
    delete entry; // discard the unregistered entry from open()
    return registered;
}

// ============================================================
// Test result tracking
// ============================================================
int tests_passed = 0;
int tests_failed = 0;

void check(bool condition, const char* test_name) {
    if (condition) {
        printf("  PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("  FAIL: %s\n", test_name);
        tests_failed++;
    }
}

// ============================================================
// 1. Single-threaded unit tests
// ============================================================

void test_create() {
    printf("\n=== Test: create() ===\n");
    FileSystem fs;

    bool r1 = fs.create("file1", 2);
    check(r1 == true, "create file1 (2 blocks) succeeds");

    bool r2 = fs.create("file1", 2);
    check(r2 == false, "duplicate create file1 fails");

    bool r3 = fs.create("file2", 3);
    check(r3 == true, "create file2 (3 blocks) succeeds");

    // Try to allocate more blocks than the disk has
    bool r4 = fs.create("huge", 600);
    check(r4 == false, "create with insufficient space fails");
}

void test_open() {
    printf("\n=== Test: open() ===\n");
    FileSystem fs;
    fs.create("file1", 2);

    ThreadFileEntry* e1 = fs.open("file1");
    check(e1 != nullptr, "open existing file returns valid entry");
    check(e1->system_entry->open_count == 1, "open count is 1 after first open");

    // Open same file again -- open count should go up
    ThreadFileEntry* e2 = fs.open("file1");
    check(e2 != nullptr, "second open returns valid entry");
    check(e2->system_entry->open_count == 2, "open count is 2 after second open");

    // Try to open a file that doesn't exist
    ThreadFileEntry* e3 = fs.open("nonexistent");
    check(e3 == nullptr, "open non-existent file returns nullptr");

    delete e1;
    delete e2;
}

void test_write_read() {
    printf("\n=== Test: write() and read() ===\n");
    FileSystem fs;
    ThreadOpenFileTable my_table;

    fs.create("file1", 2);
    ThreadFileEntry* entry = open_and_register(fs, my_table, "file1");
    check(entry != nullptr, "open file1 for write/read");

    // Write some data
    char data[] = "Hello, File System!";
    bool wr = fs.write("file1", &my_table, data, strlen(data));
    check(wr == true, "write data succeeds");

    // Read it back and compare
    char* buf = fs.read("file1", &my_table);
    check(buf != nullptr, "read returns non-null buffer");
    check(memcmp(buf, data, strlen(data)) == 0, "read data matches written data");
    delete[] buf;

    // Try to write using a table that doesn't have the file open
    ThreadOpenFileTable empty_table;
    bool wr2 = fs.write("file1", &empty_table, data, strlen(data));
    check(wr2 == false, "write to file not in thread table fails");

    fs.close("file1", &my_table);
}

void test_close() {
    printf("\n=== Test: close() ===\n");
    FileSystem fs;
    ThreadOpenFileTable table1, table2;

    fs.create("file1", 2);

    // Two threads open the same file
    ThreadFileEntry* e1 = open_and_register(fs, table1, "file1");
    ThreadFileEntry* e2 = open_and_register(fs, table2, "file1");
    check(e1 != nullptr && e2 != nullptr, "two opens succeed");
    check(e1->system_entry->open_count == 2, "open count is 2 with two openers");

    // First close -- count should drop to 1
    bool c1 = fs.close("file1", &table1);
    check(c1 == true, "first close succeeds");

    // Second close -- count drops to 0, system-wide entry removed
    bool c2 = fs.close("file1", &table2);
    check(c2 == true, "second close succeeds");

    // Try to close a file that isn't in the table
    ThreadOpenFileTable empty_table;
    bool c3 = fs.close("file1", &empty_table);
    check(c3 == false, "close file not in thread table fails");
}

void test_rename() {
    printf("\n=== Test: rename() ===\n");
    FileSystem fs;
    ThreadOpenFileTable my_table;

    fs.create("original", 2);

    bool r1 = fs.rename("original", "renamed");
    check(r1 == true, "rename succeeds");

    // Old name should no longer work
    ThreadFileEntry* e1 = fs.open("original");
    check(e1 == nullptr, "open old name fails after rename");

    // New name should work
    ThreadFileEntry* e2 = fs.open("renamed");
    check(e2 != nullptr, "open new name succeeds after rename");
    delete e2;

    // Rename a file that doesn't exist
    bool r2 = fs.rename("nonexistent", "something");
    check(r2 == false, "rename non-existent file fails");

    // Rename to a name that's already taken
    fs.create("other", 1);
    bool r3 = fs.rename("renamed", "other");
    check(r3 == false, "rename to existing name fails");
}

void test_delete() {
    printf("\n=== Test: remove() ===\n");
    FileSystem fs;
    ThreadOpenFileTable my_table;

    fs.create("file1", 2);

    // Delete a file that doesn't exist
    bool d1 = fs.remove("nonexistent");
    check(d1 == false, "delete non-existent file fails");

    // Try to delete while the file is open
    ThreadFileEntry* e1 = open_and_register(fs, my_table, "file1");
    bool d2 = fs.remove("file1");
    check(d2 == false, "delete open file fails");

    // Close it first, then delete
    fs.close("file1", &my_table);
    bool d3 = fs.remove("file1");
    check(d3 == true, "delete closed file succeeds");

    // Verify the file is gone
    ThreadFileEntry* e2 = fs.open("file1");
    check(e2 == nullptr, "open deleted file fails");
}

// ============================================================
// 2. Multi-threaded concurrency tests
// ============================================================

// Shared file system used by all concurrency tests
FileSystem* shared_fs;

// --- Test: concurrent create ---
void* thread_create(void* arg) {
    int id = *(int*)arg;
    char name[32];
    sprintf(name, "concurrent_%d", id);
    shared_fs->create(name, 1);
    return NULL;
}

void test_concurrent_create() {
    printf("\n=== Test: concurrent create() ===\n");
    shared_fs = new FileSystem();

    const int NUM_THREADS = 10;
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    // Launch 10 threads, each creating a different file
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_create, &ids[i]);
    }
    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Check that all 10 files were created successfully
    bool all_exist = true;
    for (int i = 0; i < NUM_THREADS; i++) {
        char name[32];
        sprintf(name, "concurrent_%d", i);
        ThreadFileEntry* e = shared_fs->open(name);
        if (e == nullptr) {
            all_exist = false;
        } else {
            delete e;
        }
    }
    check(all_exist, "all 10 concurrently created files exist");

    delete shared_fs;
}

// --- Test: concurrent open/close on same file ---
struct OpenCloseArgs {
    int id;
    int iterations;
};

void* thread_open_close(void* arg) {
    OpenCloseArgs* a = (OpenCloseArgs*)arg;
    for (int i = 0; i < a->iterations; i++) {
        ThreadOpenFileTable my_table;
        ThreadFileEntry* entry = shared_fs->open("shared_file");
        if (entry != nullptr) {
            my_table.add_entry("shared_file", entry->system_entry);
            delete entry;
            shared_fs->close("shared_file", &my_table);
        }
    }
    return NULL;
}

void test_concurrent_open_close() {
    printf("\n=== Test: concurrent open/close on same file ===\n");
    shared_fs = new FileSystem();
    shared_fs->create("shared_file", 2);

    const int NUM_THREADS = 8;
    const int ITERATIONS = 100;
    pthread_t threads[NUM_THREADS];
    OpenCloseArgs args[NUM_THREADS];

    // Launch 8 threads, each opening and closing the same file 100 times
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = {i, ITERATIONS};
        pthread_create(&threads[i], NULL, thread_open_close, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // After all threads finish, the file should still exist with open count back to normal
    ThreadFileEntry* e = shared_fs->open("shared_file");
    check(e != nullptr, "file still exists after concurrent open/close");
    check(e->system_entry->open_count == 1, "open count is 1 (fresh open after all closed)");
    delete e;

    delete shared_fs;
}

// --- Test: concurrent writes to same file ---
struct WriteArgs {
    int id;
    char fill_char;  // each thread writes a different character
};

void* thread_write(void* arg) {
    WriteArgs* a = (WriteArgs*)arg;
    ThreadOpenFileTable my_table;

    ThreadFileEntry* entry = shared_fs->open("write_file");
    if (entry != nullptr) {
        my_table.add_entry("write_file", entry->system_entry);
        delete entry;

        // Write 512 bytes of a single character
        char data[512];
        memset(data, a->fill_char, 512);
        shared_fs->write("write_file", &my_table, data, 512);

        shared_fs->close("write_file", &my_table);
    }
    return NULL;
}

void test_concurrent_write() {
    printf("\n=== Test: concurrent writes to same file ===\n");
    shared_fs = new FileSystem();
    shared_fs->create("write_file", 4);

    const int NUM_THREADS = 4;
    pthread_t threads[NUM_THREADS];
    WriteArgs args[NUM_THREADS];

    // Launch 4 threads, each writing a different character
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = {i, (char)('A' + i)};
        pthread_create(&threads[i], NULL, thread_write, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Read the file and check that no block has mixed data from multiple threads
    ThreadOpenFileTable my_table;
    ThreadFileEntry* entry = shared_fs->open("write_file");
    my_table.add_entry("write_file", entry->system_entry);
    delete entry;

    char* buf = shared_fs->read("write_file", &my_table);
    bool consistent = true;
    int written_chunks = 0;
    for (int chunk = 0; chunk < 4; chunk++) {
        char first = buf[chunk * 512];
        if (first == 0) continue; // this chunk wasn't written to
        written_chunks++;
        for (int j = 1; j < 512; j++) {
            if (buf[chunk * 512 + j] != first) {
                consistent = false;
                break;
            }
        }
    }
    check(consistent, "no data corruption in concurrent writes");
    printf("  INFO: %d chunks written by %d threads\n", written_chunks, NUM_THREADS);

    delete[] buf;
    shared_fs->close("write_file", &my_table);
    delete shared_fs;
}

// --- Test: mixed concurrent operations ---
void* thread_mixed_create_delete(void* arg) {
    int id = *(int*)arg;
    char name[32];
    sprintf(name, "mixed_%d", id);

    // Create a file, open it, close it, then delete it
    shared_fs->create(name, 1);
    ThreadOpenFileTable my_table;
    ThreadFileEntry* entry = shared_fs->open(name);
    if (entry != nullptr) {
        my_table.add_entry(name, entry->system_entry);
        delete entry;
        shared_fs->close(name, &my_table);
    }
    shared_fs->remove(name);
    return NULL;
}

void test_concurrent_mixed() {
    printf("\n=== Test: mixed concurrent create/open/close/delete ===\n");
    shared_fs = new FileSystem();

    const int NUM_THREADS = 10;
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_mixed_create_delete, &ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // All files should be gone after create/open/close/delete
    bool all_gone = true;
    for (int i = 0; i < NUM_THREADS; i++) {
        char name[32];
        sprintf(name, "mixed_%d", i);
        ThreadFileEntry* e = shared_fs->open(name);
        if (e != nullptr) {
            all_gone = false;
            delete e;
        }
    }
    check(all_gone, "all files deleted after concurrent create/open/close/delete");

    delete shared_fs;
}

// ============================================================
// Main
// ============================================================
int main() {
    printf("========================================\n");
    printf("  Multithreaded File System Test Suite\n");
    printf("========================================\n");

    // --- Single-threaded unit tests ---
    printf("\n--- Single-Threaded Unit Tests ---\n");
    test_create();
    test_open();
    test_write_read();
    test_close();
    test_rename();
    test_delete();

    // --- Multi-threaded concurrency tests ---
    printf("\n--- Multi-Threaded Concurrency Tests ---\n");
    test_concurrent_create();
    test_concurrent_open_close();
    test_concurrent_write();
    test_concurrent_mixed();

    // --- Summary ---
    printf("\n========================================\n");
    printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("========================================\n");

    return tests_failed > 0 ? 1 : 0;
}
