// engine functions accessible from game code
struct FileContent {
    u64 byteCount;
    void* data;
};

FileContent FileReadAll(const char path[]);
void FileWriteAll(const char path[], void* data, u64 byteCount);
void FileReleaseMemory(void* data);