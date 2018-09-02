# Filesystem

This is a header-only filesystem helper library, based closely on the
filesystem parts of C++17 but implemented for C++11 or C++14.

*This is a still work in progress, but I would call this a first candidate
for completeness. It could still use some polishing, I didn't benchmark
much yet, but I'll try to optimize some parts and refactor others.
Feedback is welcome.*


## Motivation

I'm often in need of filesystem functionality, mostly `fs::path`, but directory
access too, and when beginning to use C++11, I used that language update
to try to reduce my third-party dependencies. I could drop most of what
I used, but still missed some stuff that I started implementing for the
fun of it. Originally I based these helpers on my own coding- and naming
conventions. When C++17 was finalized, I wanted to use that interface,
but it took a while, to push myself to convert my classes.

The implementation is closely based on chapter 30.10 from the C++17 standard
and a draft close to that version is
[Working Draft N4687](https://github.com/cplusplus/draft/raw/master/papers/n4687.pdf).
It is from after the standardization of C++17 but it contains the latest filesystem
interface changes compared to the
[Working Draft N4659](https://github.com/cplusplus/draft/raw/master/papers/n4659.pdf).

I want to thank the people working on improving C++, I really liked how the language
evolved with C++11 and the following standards. Keep on the good work!


## Platforms

`ghc::filesystem` is developed on macOS but tested on Windows and Linux.
It should work on any of these with a C++11-capable compiler. I currently
don't have a BSD derivate besides macOS, so the preprocessor checks will
cry out if you try to use it there, but if there is demand, I can try to
help. Still, it shouldn't replace `std::filesystem` where full C++17 is
available, it doesn't try to be a "better" `std::filesystem`, just a drop-in
if you can't use it.

## Tests

The header comes with a set of unit-tests and uses CMake as a build tool.
If the default compiler is a GCC 8 or newer, or Clang 8 or newer, it
additionally builds a version of the test binary compiled against GCCs/Clangs
`std::filesystem` implementation,
as an additional test of conformance. Ideally all tests should compile and
succeed with all filesystem implementations, but in reality, there are
some differences in behavior.

## Usage

As it is a header-only library, it should be enough to copy the header
into your project folder oder point your include path to this directory and
simply include the `filesystem.h` header.

Everything is in the namespace `ghc::filesystem`, so one way to use it could be:

```cpp
#if defined(__cplusplus) && __cplusplus >= 201703L
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "filesystem.h"
namespace fs = ghc::filesystem;
#endif
```

If you are paranoid you can add the feature tests of C++17 to ensure your compiler
already has std::filesystem when using `-std=c++17`:

```cpp
#if defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "filesystem.h"
namespace fs = ghc::filesystem;
#endif
```

Be aware, as a header-only library, it is not hiding the fact, that it
uses system includes, so they "pollute" your global namespace.


## Documentation

There is no documentation in this release, as any `std::filesystem` documentation
would work, besides the few differences explained in the next section. So you might
head over to https://en.cppreference.com/w/cpp/filesystem for a description of
the components of this library.


## Differences

As this implementation is based on existing code from my private helper
classes, it derived some constraints of it, leading to some differences
between this and the standard C++17 API.

### LWG Defects

This implementation has switchable behavior for the LWG defects
[#2935](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2935) and
[#2937](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2937).
The currently selected behavior is following
[#2937](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2937) but
not following [#2935](http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2935),
as I feel it is a bug to report no error on a `create_directory()` or `create_directories()`
where a regular file of the same name prohibits the creation of a directory and forces
the user of those functions to double-check via `fs::is_directory` if it really worked.

### Not Implemented

Besides this still being work-in-progress, there are a few cases where
there will be no implementation in the close future:

```cpp
// methods in path:
path& operator+=(basic_string_view<value_type> x);
int compare(basic_string_view<value_type> s) const;
```

These are not implemented, as there is no `std::basic_string_view` available in
C++11 and I did want to keep this implementation self-contained and not
write a full C++17-upgrade for C++11.


### Differences in API

```cpp
filesystem::path::string_type
filesystem::path::value_type
```

In Windows, an implementation should use `std::wstring` and `wchar_t` as types used
for the native representation, but as I'm a big fan of the
["UTF-8 Everywhere" philosophy](https://utf8everywhere.org/), I decided
agains it for now. If you need to call some Windows API, use the W-variant
with the `path::wstring()` member
(e.g. `GetFileAttributesW(p.wstring().c_str())`). This gives you the
Unicode variant independant of the `UNICODE` macro and makes sharing code
between Windows, Linux and macOS easier.

```cpp
const path::string_type& path::native() const /*noexcept*/;
const path::value_type *path::c_str() const /*noexcept*/;
```

These two can not be `noexcept` with the current implementation. This due
to the fact, that internally path is working on the generic path version
only, and the getters need to do a conversion to native path format.

```cpp
const path::string_type& path::generic_string() const;
```

This returns a const reference, instead of a value, because it can. This
implementation uses the generic representation for internal workings, so
it's "free" to return that.


### Differences in Behavior

#### fs.path

As the complete inner mechanics of this implementation `fs::path` are working
on the generic format, it is the internal representation. So creating any mixed
slash `fs::path` object under Windows (e.g. with `"C:\foo/bar"`) will lead to a
unified path with `"C:\foo\bar"` via `native()` and `"C:/foo/bar"` via
`generic_string()` API.

Additionally this implementation follows the standards suggestion to handle
posix paths of the form `"//host/path"` and USC path on windows also as having
a root-name (e.g. `"//host"`). The GCC implementation didn't choose to do that
while testing on Ubuntu 18.04 and macOS with GCC 8.1.0 or Clang 8. This difference will
show as warnings under std::filesystem. This leads to a change in the
algorithm described in the standard for `operator/=(path& p)` where any path
`p` with `p.is_absolute()` will degrade to an assignment, while this implementation
has the exception where `*this == *this.root_name()` and `p == preferred_seperator`
a normal append will be done, to allow:

```cpp
fs::path p1 = "//host/foo/bar/file.txt";
fs::path p2;
for (auto p : p1) p2 /= p;
ASSERT(p1 == p2);
```

For all non-host-leading paths the behaviour will match the one described by
the standard.

#### fs.op.copy

Then there is `fs::copy`. The tests in the suite fail partially with C++17 `std::filesystem`
on GCC/Clang. They complain about a copy call with `fs::copy_options::recursive` combined
with `fs::copy_options::create_symlinks` or `fs::copy_options::create_hard_links` if the
source is a directory. There is nothing in the standard that forbids this combination
and it is the only way to deep-copy a tree while only create links for the files.
There is [LWG #2682](https://wg21.cmeerw.net/lwg/issue2682) that supports this
interpretation, but the issue ignores the usefulness of the combination with recursive
and part of the justification for the proposed solution is "we did it so for almost two years".
But this makes `fs::copy` with `fs::copy_options::create_symlinks` or `fs::copy_options::create_hard_links`
just a more complicated syntax for the `fs::create_symlink` or `fs::create_hardlink` operation
and I don't want to believe, that this was the intention of the original writing.
As there is another issue related to copy, with a different take on the description,
I keep my version the way I read the description, as it is not contradicting the standard and useful. Let's see
what final solution the LWG comes up with in the end.


## Open Issues

### Windows

#### Symbolic Links

As symbolic links on Windows, while being supported more or less since
Windows Vista (with some strict security constraints) and fully since some earlier
build of Windows 10, when "Developer Mode" is activated, are at time of writing
(2018) rarely used, still they are supported with this implementation.

#### Permissions

The Windows ACL permission feature translates badly to the POSIX permission
bit mask used in the interface of C++17 filesystem. The permissions returned
in the `file_status` are therefore currently synthesized for the `user`-level
and copied to the `group`- and `other`-level. There is still some potential
for more interaction with the Windows permission system, but currently setting
or reading permissions with this implementation will most certainly not lead
to the expected behavior.

