#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "FileSystem.h"
#include "ptOFT.h"

// ============================================================
// Helper: open a file and register it in a per-thread ptOFT
// ============================================================
ptOFT_Entry* open_and_register(FileSystem& fs, ptOFT& table, string file_name) {
    ptOFT_Entry* pt = fs.open(file_name);
    if (pt == nullptr) return nullptr;
    // add to per-thread table using the swOFT pointer from open()
    ptOFT_Entry* registered = table.add_entry(file_name, pt->swOFT_pointer);
    delete pt; // discard the unregistered entry from open()
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

    // Allocate too many blocks to exhaust space
    bool r4 = fs.create("huge", 600);
    check(r4 == false, "create with insufficient space fails");
}

void test_open() {
    printf("\n=== Test: open() ===\n");
    FileSystem fs;
    fs.create("file1", 2);

    ptOFT_Entry* e1 = fs.open("file1");
    check(e1 != nullptr, "open existing file returns valid entry");
    check(e1->swOFT_pointer->reference_count == 1, "reference count is 1 after first open");

    // Open same file again — reference count should increment
    ptOFT_Entry* e2 = fs.open("file1");
    check(e2 != nullptr, "second open returns valid entry");
    check(e2->swOFT_pointer->reference_count == 2, "reference count is 2 after second open");

    // Open non-existent file
    ptOFT_Entry* e3 = fs.open("nonexistent");
    check(e3 == nullptr, "open non-existent file returns nullptr");

    delete e1;
    delete e2;
}

void test_write_read() {
    printf("\n=== Test: write() and read() ===\n");
    FileSystem fs;
    ptOFT my_table;

    fs.create("file1", 2);
    ptOFT_Entry* entry = open_and_register(fs, my_table, "file1");
    check(entry != nullptr, "open file1 for write/read");

    // Write data
    char data[] = "Hello, File System!";
    bool wr = fs.write("file1", &my_table, data, strlen(data));
    check(wr == true, "write data succeeds");

    // Read data back
    char* buf = fs.read("file1", &my_table);
    check(buf != nullptr, "read returns non-null buffer");
    check(memcmp(buf, data, strlen(data)) == 0, "read data matches written data");
    delete[] buf;

    // Write to unopened file
    ptOFT empty_table;
    bool wr2 = fs.write("file1", &empty_table, data, strlen(data));
    check(wr2 == false, "write to file not in ptOFT fails");

    fs.close("file1", &my_table);
}

void test_close() {
    printf("\n=== Test: close() ===\n");
    FileSystem fs;
    ptOFT table1, table2;

    fs.create("file1", 2);

    // Two threads open the same file
    ptOFT_Entry* e1 = open_and_register(fs, table1, "file1");
    ptOFT_Entry* e2 = open_and_register(fs, table2, "file1");
    check(e1 != nullptr && e2 != nullptr, "two opens succeed");
    check(e1->swOFT_pointer->reference_count == 2, "ref count is 2 with two openers");

    // First close — ref count should drop to 1
    bool c1 = fs.close("file1", &table1);
    check(c1 == true, "first close succeeds");

    // Second close — ref count drops to 0, swOFT entry removed
    bool c2 = fs.close("file1", &table2);
    check(c2 == true, "second close succeeds");

    // Close file not in table
    ptOFT empty_table;
    bool c3 = fs.close("file1", &empty_table);
    check(c3 == false, "close file not in ptOFT fails");
}

void test_rename() {
    printf("\n=== Test: rename() ===\n");
    FileSystem fs;
    ptOFT my_table;

    fs.create("original", 2);

    bool r1 = fs.rename("original", "renamed");
    check(r1 == true, "rename succeeds");

    // Old name should no longer be openable
    ptOFT_Entry* e1 = fs.open("original");
    check(e1 == nullptr, "open old name fails after rename");

    // New name should be openable
    ptOFT_Entry* e2 = fs.open("renamed");
    check(e2 != nullptr, "open new name succeeds after rename");
    delete e2;

    // Rename non-existent file
    bool r2 = fs.rename("nonexistent", "something");
    check(r2 == false, "rename non-existent file fails");

    // Rename to existing name
    fs.create("other", 1);
    bool r3 = fs.rename("renamed", "other");
    check(r3 == false, "rename to existing name fails");
}

void test_delete() {
    printf("\n=== Test: remove() ===\n");
    FileSystem fs;
    ptOFT my_table;

    fs.create("file1", 2);

    // Delete non-existent
    bool d1 = fs.remove("nonexistent");
    check(d1 == false, "delete non-existent file fails");

    // Delete while file is open
    ptOFT_Entry* e1 = open_and_register(fs, my_table, "file1");
    bool d2 = fs.remove("file1");
    check(d2 == false, "delete open file fails");

    // Close then delete
    fs.close("file1", &my_table);
    bool d3 = fs.remove("file1");
    check(d3 == true, "delete closed file succeeds");

    // Verify file is gone
    ptOFT_Entry* e2 = fs.open("file1");
    check(e2 == nullptr, "open deleted file fails");
}

// ============================================================
// 2. Multi-threaded concurrency tests
// ============================================================

// Shared file system for concurrency tests
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

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_create, &ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Verify all files were created
    bool all_exist = true;
    for (int i = 0; i < NUM_THREADS; i++) {
        char name[32];
        sprintf(name, "concurrent_%d", i);
        ptOFT_Entry* e = shared_fs->open(name);
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
        ptOFT my_table;
        ptOFT_Entry* pt = shared_fs->open("shared_file");
        if (pt != nullptr) {
            my_table.add_entry("shared_file", pt->swOFT_pointer);
            delete pt;
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

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = {i, ITERATIONS};
        pthread_create(&threads[i], NULL, thread_open_close, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // After all opens/closes, file should still exist and ref count should be 0
    // (no swOFT entry since all threads closed)
    ptOFT_Entry* e = shared_fs->open("shared_file");
    check(e != nullptr, "file still exists after concurrent open/close");
    check(e->swOFT_pointer->reference_count == 1, "ref count is 1 (fresh open after all closed)");
    delete e;

    delete shared_fs;
}

// --- Test: concurrent writes to same file ---
struct WriteArgs {
    int id;
    char fill_char;
};

void* thread_write(void* arg) {
    WriteArgs* a = (WriteArgs*)arg;
    ptOFT my_table;

    ptOFT_Entry* pt = shared_fs->open("write_file");
    if (pt != nullptr) {
        my_table.add_entry("write_file", pt->swOFT_pointer);
        delete pt;

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

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i] = {i, (char)('A' + i)};
        pthread_create(&threads[i], NULL, thread_write, &args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Read the file and verify no corruption (each 512-byte chunk should
    // contain a single character, no mixed data within a chunk)
    ptOFT my_table;
    ptOFT_Entry* pt = shared_fs->open("write_file");
    my_table.add_entry("write_file", pt->swOFT_pointer);
    delete pt;

    char* buf = shared_fs->read("write_file", &my_table);
    bool consistent = true;
    // Check that we got valid data (non-zero, since threads wrote)
    int written_chunks = 0;
    for (int chunk = 0; chunk < 4; chunk++) {
        char first = buf[chunk * 512];
        if (first == 0) continue; // unwritten chunk
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

    shared_fs->create(name, 1);
    // Open and close
    ptOFT my_table;
    ptOFT_Entry* pt = shared_fs->open(name);
    if (pt != nullptr) {
        my_table.add_entry(name, pt->swOFT_pointer);
        delete pt;
        shared_fs->close(name, &my_table);
    }
    // Delete
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

    // All files should be deleted — verify by trying to open them
    bool all_gone = true;
    for (int i = 0; i < NUM_THREADS; i++) {
        char name[32];
        sprintf(name, "mixed_%d", i);
        ptOFT_Entry* e = shared_fs->open(name);
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
