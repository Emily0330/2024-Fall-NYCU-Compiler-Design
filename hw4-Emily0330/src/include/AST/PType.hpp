#ifndef AST_P_TYPE_H
#define AST_P_TYPE_H

#include <memory>
#include <string>
#include <vector>

class PType;

using PTypeSharedPtr = std::shared_ptr<PType>;

class PType {
  public:
    enum class PrimitiveTypeEnum : uint8_t {
        kVoidType,
        kIntegerType,
        kRealType,
        kBoolType,
        kStringType
    };

  private:
    PrimitiveTypeEnum m_type;
    std::vector<uint64_t> m_dimensions;
    mutable std::string m_type_string;
    mutable bool m_type_string_is_valid = false;

  public:
    ~PType() = default;
    PType(const PrimitiveTypeEnum type) : m_type(type) {}

    void setDimensions(std::vector<uint64_t> &p_dims) {
        m_dimensions = std::move(p_dims);
    }
    
    const uint8_t getDimension() const { return m_dimensions.size(); } // get the number of dimensions
    std::vector<uint64_t> getDimensions() const { return m_dimensions; }

    PTypeSharedPtr getArrayReferenceType(uint64_t dims) const {
        // Already checked outside function
        // if (dims > m_dimensions.size())
        // {
        //     return nullptr;
        // }

        PTypeSharedPtr type = std::make_shared<PType>(m_type);
        type->m_dimensions = std::vector<uint64_t>(m_dimensions.begin() + dims, m_dimensions.end());

        return type;
    }

    PrimitiveTypeEnum getPrimitiveType() const { return m_type; }
    const char *getPTypeCString() const;
    bool isInteger() const { return (m_type == PrimitiveTypeEnum::kIntegerType); }
    bool isReal() const { return (m_type == PrimitiveTypeEnum::kRealType); }
    bool isBoolean() const { return (m_type == PrimitiveTypeEnum::kBoolType); }
    bool isString() const { return (m_type == PrimitiveTypeEnum::kStringType); }

    bool isABool() const { return isBoolean() && getDimension() == 0; } // is a bool (not bool array)
    // bool isAnInteger() const { return isBoolean() && getDimension() == 0; } // is a bool (not bool array)
    // bool isAString() const { return isBoolean() && getDimension() == 0; } // is a bool (not bool array)
    // bool isAReal() const { return isBoolean() && getDimension() == 0; } // is a bool (not bool array)

    bool isScalar() const { return getDimension() == 0 && (isInteger() || isReal() || isBoolean() || isString()); }

    bool isSame(PTypeSharedPtr B) const {
      if(this->getPrimitiveType() != B->getPrimitiveType()) return false;
      if(this->m_dimensions != B->getDimensions()) return false;
      return true;
    }

    bool isSameAfterThisCoercion(PTypeSharedPtr B) const { // if "this is integer, B is real" should be fine (cannot be array)
      if(this->isInteger() && B->isReal() && this->getDimension() == 0); // do nothing
      else if(this->getPrimitiveType() != B->getPrimitiveType()) return false;
      if(this->m_dimensions != B->getDimensions()) return false;
      return true;
    }
};

#endif
