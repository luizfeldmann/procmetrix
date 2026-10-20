#ifndef PROCMETRIX_NODEJS_REFLECTION_H
#define PROCMETRIX_NODEJS_REFLECTION_H

// Node N-API
#include <napi.h>

//! Represents a the information of a member of an object
template <typename object_type, typename member_type>
struct member_field
{
    using member_type_t = member_type;
    using member_ptr_t = member_type object_type::*;

    //! Name of the member
    const char* name;

    //! Pointer to the member
    member_ptr_t member;
};

//! Must be specialized to implement reflection for a particular type;
template <typename object_type>
struct reflection;

//! Serialization visitor
template <typename object_type>
struct serialize_visitor
{
    //! The object being serialized
    object_type const& m_input;

    //! Receives the serialized data
    Napi::Object& m_output;

    //! Constructs the visitor wrapping the object
    serialize_visitor(object_type const& input, Napi::Object& output)
        : m_input(input)
        , m_output(output)
    {
    }

    //! Apply the visitor to the input data
    template <typename member_type>
    void operator()(member_type const& member_info)
    {
        m_output.Set(member_info.name, m_input.*(member_info.member));
    }
};

//! Converts the native object to a Node object
template <typename object_type>
Napi::Object serialize(Napi::Env env, object_type const& input)
{
    Napi::Object output = Napi::Object::New(env);
    reflection<object_type>::accept(
        serialize_visitor<object_type>(input, output));

    return output;
}

//! Parses the JS value as the given type
template <typename T>
T parse_value(Napi::Value const& value);

//! Parses the value as double
template <>
inline double parse_value<double>(Napi::Value const& value)
{
    return value.As<Napi::Number>().DoubleValue();
}

//! Parsing visitor
template <typename object_type>
struct parse_visitor
{
    //! The object being parsed
    Napi::Object const& m_input;

    //! Receives the parsed data
    object_type& m_output;

    //! Constructs the visitor wrapping the object
    parse_visitor(Napi::Object const& input, object_type& output)
        : m_input(input)
        , m_output(output)
    {
    }

    //! Apply the visitor to the input data
    template <typename member_info_type>
    void operator()(member_info_type const& member_info)
    {
        using member_type = typename member_info_type::member_type_t;
        m_output.*(member_info.member) =
            parse_value<member_type>(m_input.Get(member_info.name));
    }
};

//! Converts a Node object to a native object
template <typename object_type>
void parse(Napi::Object const& input, object_type& output)
{
    reflection<object_type>::accept(parse_visitor<object_type>(input, output));
}

#endif
