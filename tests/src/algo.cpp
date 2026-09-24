// Private impl
#include <internal/algo.h>

// Testing
#include <gtest/gtest.h>

// Test cases

/** Null tokenizer */

TEST(procmetrix_split_zero_terminated_tokens, null_args)
{
    // Null inputs
    size_t num_tokens = 0;
    char** tokens = nullptr;
 
    EXPECT_EQ(
        procmetrix_split_zero_terminated_tokens(nullptr, 0, &tokens, &num_tokens),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
        
    EXPECT_EQ(tokens, nullptr);
    EXPECT_EQ(num_tokens, 0);
        
    // Null outputs
    const char buffer[]{"hello world"};
 
    EXPECT_EQ(
        procmetrix_split_zero_terminated_tokens(buffer, std::size(buffer), nullptr, nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_split_zero_terminated_tokens, empty)
{
    // Empty input
    const char buffer[]{""};

    // Finds a single token o zero size
    size_t num_tokens = 0;
    char** tokens = nullptr;

    EXPECT_EQ(
        procmetrix_split_zero_terminated_tokens(buffer, std::size(buffer), &tokens, &num_tokens),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(num_tokens, 1);
    EXPECT_NE(tokens, nullptr);

    if (nullptr != tokens && 0 != num_tokens)
        EXPECT_STREQ(tokens[0], "");

    // Cleanup
    for (size_t i = 0; i < num_tokens; ++i)
        free(tokens[i]);
    free(tokens);
}

TEST(procmetrix_split_zero_terminated_tokens, trivial)
{
    const char buffer[]{"a\0b"};

    size_t num_tokens = 0;
    char** tokens = nullptr;

    EXPECT_EQ(
        procmetrix_split_zero_terminated_tokens(buffer, std::size(buffer), &tokens, &num_tokens),
        PROCMETRIX_ERROR_NONE);

    // Expect result ["a", "b"]
    EXPECT_EQ(num_tokens, 2);
    EXPECT_NE(tokens, nullptr);

    // Check actual values
    if (nullptr != tokens && num_tokens > 0)
        EXPECT_STREQ(tokens[0], "a");

    if (nullptr != tokens && num_tokens > 1)
        EXPECT_STREQ(tokens[1], "b");

    // Cleanup
    for (size_t i = 0; i < num_tokens; ++i)
        free(tokens[i]);
    free(tokens);
}

/** Env var tokenizer */

TEST(procmetrix_split_environ_vars, null_args)
{
    // Null inputs
    procmetrix_proc_environ_t proc_environ { 0 };

    EXPECT_EQ(
        procmetrix_split_environ_vars(nullptr, 0, &proc_environ),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);

    EXPECT_EQ(proc_environ.count, 0);
    EXPECT_EQ(proc_environ.vars, nullptr);

    // Null output
    const char* const vars[] {
        "hello=world",
        "foo=bar"
    };

    EXPECT_EQ(
        procmetrix_split_environ_vars(vars, std::size(vars), nullptr),
        PROCMETRIX_ERROR_INVALID_ARGUMENT);
}

TEST(procmetrix_split_environ_vars, trivial)
{
    // Parse trivial environment
    procmetrix_proc_environ_t proc_environ { 0 };
    const char* const vars[] {
        // Regular
        "hello=world",
        "foo=bar",
        // No right side (no value)
        "empty=",
        // No delimiter
        "flag",
    };

    EXPECT_EQ(
        procmetrix_split_environ_vars(vars, std::size(vars), &proc_environ),
        PROCMETRIX_ERROR_NONE);

    EXPECT_EQ(proc_environ.count, 4);
    EXPECT_NE(proc_environ.vars, nullptr);

    // Check the actual values
    if (nullptr != proc_environ.vars && proc_environ.count > 0)
    {
        EXPECT_STREQ(proc_environ.vars[0].name, "hello");
        EXPECT_STREQ(proc_environ.vars[0].value, "world");
    }

    if (nullptr != proc_environ.vars && proc_environ.count > 1)
    {
        EXPECT_STREQ(proc_environ.vars[1].name, "foo");
        EXPECT_STREQ(proc_environ.vars[1].value, "bar");
    }

    if (nullptr != proc_environ.vars && proc_environ.count > 2)
    {
        EXPECT_STREQ(proc_environ.vars[2].name, "empty");
        EXPECT_STREQ(proc_environ.vars[2].value, "");
    }

    if (nullptr != proc_environ.vars && proc_environ.count > 3)
    {
        EXPECT_STREQ(proc_environ.vars[3].name, "flag");
        EXPECT_EQ(proc_environ.vars[3].value, nullptr);
    }

    // Cleanup
    procmetrix_free_proc_environ(&proc_environ);
}
