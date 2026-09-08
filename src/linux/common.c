// Internal
#include <internal/linux/common_linux_internal.h>

// Linux
#include <fcntl.h>
#include <unistd.h>

// Impl

FILE *procmetrix_impl_linux_open_file_rdonly_cloexec(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return NULL;

#ifdef FD_CLOEXEC
    fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif

    FILE *fp = fdopen(fd, "r");
    if (fp == NULL)
        close(fd);

    return fp;
}
