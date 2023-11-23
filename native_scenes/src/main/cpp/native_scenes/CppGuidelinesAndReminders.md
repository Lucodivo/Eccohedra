### Guidelines
Disclaimer: Guidelines are a suggested general way of doing things. There are many reasons to break rules. 
These are no exception.

#### Function arguments: Pointer or reference?
- Although it may seem arbitrary, choosing a reference or a pointer can change the perception of what 
a function may do at the calling site. Pass-by-reference looks just like pass-by-value at the calling site. 
Preferably the calling site screams "This function may mutate your argument!" So..
    - Does the function mutate the argument?
        - "Yes", "Maybe", or "Not now but I want to keep the possibilities open": The parameter should be a pointer.
        - "No": The parameter should be a reference to a constant value

### Wait... How does this work again?
#### Const (and pointers)
- General tip: "Read it backwards."
- Examples:
	- `int*` - pointer to an int
	- `int const *` - pointer to a const int
	- `int* const` - const pointer to int
	- `int const * const` - const pointer to const int
- However, the first `const` can be on either side of type, so:
	- `const int*` == `int const *`
	- `const int* const` == `int const * const`
- Extreme examples:
	- `int const * const * * const` - const pointer to pointer to const pointer to const int
	- `const int * const **` - pointer to pointer to const pointer to const int
#### How do I parse this declaration?
- [The "Clockwise/Spiral Rule" by David Anderson](http://c-faq.com/decl/spiral.anderson.html) 

#### Weird/Dumb user errors
- Might look like: *ld: error: undefined symbol:*
  - Linker problem. Can't find a function plainly written in a header file of a static library that 
  compiles without error.
  - specific example: *ld: error: undefined symbol: assets::loadAssetFile(AAssetManager*, char const*, assets::AssetFile*)*
    - cause: I had an *#include <android/asset_manager.h>* within the *assets* namespace. I believe 
    this changed the type of *AAssetManager* to *assets::AAssetManager*.
    - lesson: Be intentional with *#include* inside of a namespace. There seems to be potential use cases
    for such a thing, but it was a morale breaking for me on the day of writing this.