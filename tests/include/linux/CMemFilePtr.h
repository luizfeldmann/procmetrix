#ifndef _CMEM_FILE_PTR_H_
#define _CMEM_FILE_PTR_H_

// STD
#include <cstdio>
#include <cstring>

//! RAII for a memory file descriptor
class CMemFilePtr
{
private:
    //! Internal file pointer
    FILE* m_fp;

    //! Non copy-constructible
    CMemFilePtr(CMemFilePtr const&) = delete;

    //! Non copy-assignable
    CMemFilePtr& operator=(CMemFilePtr const&) = delete;

public:
    //! Constructor
    inline CMemFilePtr(const char* szText, size_t uLen)
        : m_fp(fmemopen((void*)szText, uLen, "r"))
    {
    }

    //! Constructor
    inline CMemFilePtr(const char* szText)
        : CMemFilePtr(szText, strlen(szText))
    {

    }

    //! Destructor
    inline ~CMemFilePtr()
    {
        fclose(m_fp);
    }

    //! Gets the native file pointer
    FILE* get() const
    {
        return m_fp;
    }
};

#endif // _CMEM_FILE_PTR_H_
