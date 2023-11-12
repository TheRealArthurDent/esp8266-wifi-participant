#pragma once

#define DECLARE_CLASS_AS_INTERFACE(ClassName) /* NOLINT(cppcoreguidelines-macro-usage) */                  \
public:                                                                                                    \
    virtual ~ClassName() {} /* NOLINT(bugprone-macro-parentheses) */                                       \
                                                                                                           \
protected:                                                                                                 \
    ClassName() {}                                                /* NOLINT(bugprone-macro-parentheses) */ \
    ClassName(const ClassName &) {}                               /* NOLINT(bugprone-macro-parentheses) */ \
    ClassName &operator=(const ClassName &) { return *this; }     /* NOLINT(bugprone-macro-parentheses) */ \
    ClassName(ClassName &&) noexcept {}                           /* NOLINT(bugprone-macro-parentheses) */ \
    ClassName &operator=(ClassName &&) noexcept { return *this; } /* NOLINT(bugprone-macro-parentheses) */ \
                                                                                                           \
private: // NOLINT(cppcoreguidelines-macro-usage,bugprone-macro-parentheses,modernize-use-trailing-return-type)
