#ifndef _CTEMP_DIR_GLOB_H_
#define _CTEMP_DIR_GLOB_H_

// STD
#include <string>
#include <fstream>

// Linux
#include <glob.h>
#include <ftw.h>
#include <unistd.h>

//! RAII for a temp directory and corresponding glob of files
class CTempDirGlob
{
private:
    //! Temp directory base
    std::string m_tempdir;

    //! Iterator callback to remove the temp files
    static int Remove(const char* path, const struct stat*, int, struct FTW*)
    {
        return remove(path);
    }

    //! Returns the full path of a file in the temp directory
    std::string MakePath(std::string subpath) {
        return m_tempdir + "/" + subpath;
    }

public:
    //! Constructor
    CTempDirGlob()
    {
        char szTemplate[] {"/tmp/procmetrix-test-XXXXXX"};
        m_tempdir = mkdtemp(szTemplate);
    }

    //! Destructor
    ~CTempDirGlob()
    {
        nftw(m_tempdir.c_str(), &CTempDirGlob::Remove, 64, FTW_DEPTH | FTW_PHYS);
    }

    //! Creates a subdirectory inside the temp directory
    //! @return True on success
    bool CreateDir(std::string subpath) {
        std::string path = MakePath(subpath);
        return 0 == mkdir(path.c_str(), 0755);
    }

    //! Writes data in the temp files
    //! True on success
    bool WriteFile(std::string subpath, std::string data) {
        std::ofstream ofs(MakePath(subpath));
        if (!ofs.is_open())
            return false;
        ofs << data;
        return true;
    }

    //! Globs the temp dir
    //! @return True on success
    int Glob(glob_t* g)
    {
        std::string pattern = m_tempdir + "/*";
        return 0 == glob(pattern.c_str(), 0, NULL, g);
    }
};

#endif // _CMOCK_GLOB_H_
