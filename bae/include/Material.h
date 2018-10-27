#pragma once
#include "MaterialType.h"

namespace bae
{

class Material
{
  public:
    Material();
    ~Material();

  private:
    MaterialType &m_matType;
};
} // namespace bae
