#ifndef CTEMP_DIR_GLOB_H
#define CTEMP_DIR_GLOB_H

// STD
#include <fstream>
#include <string>

// Linux
#include <ftw.h>
#include <glob.h>
#include <unistd.h>

//! RAII for a temp directory and corresponding glob of files
class temp_dir_glob
{
private:
    //! Temp directory base
    std::string tempdir_;

    //! Iterator callback to remove the temp files
    static int remove_cb(
        const char* path,
        const struct stat* /*statbuf*/,
        int /*typeflag*/,
        struct FTW* /*ftwbuf*/)
    {
        return remove(path);
    }

    //! Returns the full path of a file in the temp directory
    std::string make_path(std::string const& subpath)
    {
        return tempdir_ + "/" + subpath;
    }

public:
    //! Constructor
    temp_dir_glob()
    {
        char temp_template[] { "/tmp/procmetrix-test-XXXXXX" };
        tempdir_ = mkdtemp(temp_template);
    }

    //! Destructor
    ~temp_dir_glob()
    {
        nftw(
            tempdir_.c_str(),
            &temp_dir_glob::remove_cb,
            64,
            FTW_DEPTH | FTW_PHYS);
    }

    //! Creates a subdirectory inside the temp directory
    //! @return True on success
    bool create_dir(std::string const& subpath)
    {
        std::string path = make_path(subpath);
        return 0 == mkdir(path.c_str(), 0755);
    }

    //! Writes data in the temp files
    //! True on success
    bool write_file(std::string const& subpath, std::string const& data)
    {
        std::ofstream ofs(make_path(subpath));
        if (!ofs.is_open())
            return false;
        ofs << data;
        return true;
    }

    //! Globs the temp dir
    //! @return True on success
    bool glob(glob_t* glb)
    {
        std::string pattern = tempdir_ + "/*";
        return 0 == ::glob(pattern.c_str(), 0, NULL, glb);
    }
};

#endif // CTEMP_DIR_GLOB_H
