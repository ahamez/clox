#pragma once

#include <unordered_map>

#include "clox/chunk_fwd.hh"

namespace clox {

// ---------------------------------------------------------------------------------------------- //

struct ChunkContext
{
  std::unordered_map<std::string, GlobalVariableIndex> global_variables_{};
  GlobalVariableIndex last_global_variable_index_{.index = 0};

  [[nodiscard]] GlobalVariableIndex maybe_add_global_variable(const std::string&);
  [[nodiscard]] std::string get_global_variable(GlobalVariableIndex) const;
  [[nodiscard]] GlobalVariableIndex last_global_variable_index() const noexcept;
};

// ---------------------------------------------------------------------------------------------- //

} // namespace clox
