# Clox

A C++ version of the interpreter shown in the book "Crafting Interpreters."

## Differences

* Most obvious one: C++ 😉
* Opcodes are stored with a std::variant<>, rather than with raw bytes
    * It's quite nice as we can dispatch the evaluation of the operands using strong typing
    * But, as it's a discriminated union, its size is the size of the largest operand, in this case 4 bytes, which is
      quite large 😬
* Objects are stored in list specific to each type of object, rather than a single list for all types of objects.
