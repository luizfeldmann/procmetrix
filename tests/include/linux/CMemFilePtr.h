#ifndef CMEM_FILE_PTR_H
#define CMEM_FILE_PTR_H

// STD
#include <cstdio>
#include <cstring>

//! RAII for a memory file descriptor
class mem_file_ptr
{
private:
    //! Internal file pointer
    FILE* fp_;

    //! Non copy-constructible
    mem_file_ptr(mem_file_ptr const&) = delete;

    //! Non copy-assignable
    mem_file_ptr& operator=(mem_file_ptr const&) = delete;

public:
    //! Constructor
    mem_file_ptr(const char* text, size_t len)
        : fp_(fmemopen((void*)text, len, "r"))
    {
    }

    //! Constructor
    mem_file_ptr(const char* text)
        : mem_file_ptr(text, strlen(text))
    {

    }

    //! Destructor
    ~mem_file_ptr()
    {
        fclose(fp_);
    }

    //! Gets the native file pointer
    FILE* get() const
    {
        return fp_;
    }
};

#endif // CMEM_FILE_PTR_H
