///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <cassert>

#include <harbour/request/forms.hpp>

#define EXPECT(ok) \
    assert((ok));  \
    if (!(ok)) return 1;

static const std::string form_data = "name=bob&title=dude&age=32";

auto main() -> int {
    auto data = harbour::request::detail::FormDataParser(form_data).parse();
    EXPECT(data.size() == 3);
    EXPECT(data["name"] == "bob");
    EXPECT(data["title"] == "dude");
    EXPECT(data["age"] == "32");

    return 0;
}