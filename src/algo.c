// Lib
#include <internal/algo.h>

// STD
#include <stdlib.h>
#include <string.h>

// OS macros

static char *procmetrix_strndup(const char *str, size_t len)
{
#ifdef _WIN32
    if (str == NULL)
        return NULL;

    size_t new_len = strnlen_s(str, len);
    char *new_str = (char *)malloc(new_len + 1);
    if (new_str)
    {
        memcpy(new_str, str, new_len);
        new_str[new_len] = '\0';
    }
    return new_str;
#else
    return strndup(str, len);
#endif
}

static char *procmetrix_strdup(const char *str)
{
#ifdef _MSC_VER
    return _strdup(str);
#else
    return strdup(str);
#endif
}

// Impl

procmetrix_error_t procmetrix_split_zero_terminated_tokens(const char *buffer, size_t buffer_size, char ***tokens, size_t *num_tokens)
{
    // Sanity
    if (NULL == tokens || NULL == num_tokens)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Cleanup outputs
    *tokens = NULL;
    *num_tokens = 0;

    if (NULL == buffer || 0 == buffer_size)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Count how many null terminations
    for (size_t i = 0; i < buffer_size; ++i)
    {
        if (buffer[i] == '\0' || i == buffer_size - 1)
            (*num_tokens)++;
    }

    // Allocate the output list
    *tokens = (char **)calloc(*num_tokens, sizeof(char *));
    if (NULL == *tokens)
    {
        *num_tokens = 0;
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;
    }

    // Fill out the list of tokens
    for (size_t file_idx = 0, tok_idx = 0, start_idx = 0; file_idx < buffer_size; ++file_idx)
    {
        int is_last = (int)(file_idx == buffer_size - 1);
        if (buffer[file_idx] == '\0' || is_last)
        {
            (*tokens)[tok_idx++] = procmetrix_strndup(buffer + start_idx, file_idx - start_idx + is_last);
            start_idx = file_idx + 1;
        }
    }

    return PROCMETRIX_ERROR_NONE;
}

procmetrix_error_t procmetrix_split_environ_vars(const char *const *varlines, size_t numvars, procmetrix_proc_environ_t *environment)
{
    // Sanity
    if (NULL == environment)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // Consistent results even if error
    memset(environment, 0, sizeof(*environment));

    if (NULL == varlines)
        return PROCMETRIX_ERROR_INVALID_ARGUMENT;

    // If no variables, nothing to do
    if (0 == numvars)
        return PROCMETRIX_ERROR_NONE;

    // Allocate for the key-value pairs
    environment->vars =
        (procmetrix_proc_environ_var_t *)calloc(numvars, sizeof(procmetrix_proc_environ_var_t));

    if (NULL == environment->vars)
        return PROCMETRIX_ERROR_OUT_OF_MEMORY;

    // Assign each key-value
    procmetrix_error_t status = PROCMETRIX_ERROR_NONE;

    for (size_t i = 0; i < numvars; ++i)
    {
        const char *line = varlines[i];

        // Split left and right of delimiet
        const char *delim = strchr(line, '=');

        if (NULL != delim)
        {
            environment->vars[i].name = procmetrix_strndup(line, delim - line);
            environment->vars[i].value = procmetrix_strdup(delim + 1);
        }
        else
        {
            // No right-side
            environment->vars[i].name = procmetrix_strdup(line);
        }

        // Cleanup on strdup failure
        if ((NULL == environment->vars[i].name) || ((NULL == environment->vars[i].value) && (NULL != delim)))
        {
            free(environment->vars[i].name);
            environment->vars[i].name = NULL;

            free(environment->vars[i].value);
            environment->vars[i].value = NULL;

            status = PROCMETRIX_ERROR_OUT_OF_MEMORY;
            break;
        }

        // Count filled items
        environment->count = i + 1;
    }

    return status;
}
