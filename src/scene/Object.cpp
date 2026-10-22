/**
 * @file Object.cpp
 * @brief Implementation of Object class.
 */

#include "Object.hpp"

namespace Engine {

    Object::Object() : m_UUID(), m_Name("Object") {}

    Object::Object(std::string name) : m_UUID(), m_Name(std::move(name)) {}

} // namespace Engine
