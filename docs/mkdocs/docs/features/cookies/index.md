# Cookies

Harbour comes with its own cookie storage API to fasciliate a seamless way to create and manage user sessions.

A full Cookies example can be found [here](https://github.com/griefzz/harbour/blob/main/examples/cookies.cpp).

## Setting Cookies

If you want to set the cookies for a client, you need to create a [Cookies](https://github.com/griefzz/harbour/blob/main/include/harbour/cookies/cookies.hpp) object and then add it
to a Response.

!!! example

    This is an example that creates an empty cookies object and inserts some data into it.

    Then creates a response that you can send to the client setting their cookies

    ```cpp
        // Create an empty Cookies object
        auto cookies = Cookies::create();

        // The Cookies object acts as a map<string, string>
        cookies["id"]   = "123";
        cookies["name"] = "bob";

        // Create a Response with cookies
        auto response = Response().with_cookies(cookies);
    ```

!!! note

    While Harbour by default will construct a Response with a http::Status::OK. 
    The Set-Cookie header will only be valid if Response has http::Status::OK.

## Getting Cookies

In order to parse the cookies from the client we need to create a [Cookies](https://github.com/griefzz/harbour/blob/main/include/harbour/cookies/cookies.hpp) object from a Request

!!! example

    This example will parse the Cookies from a Request. Check if id and name exist in the [Cookies](https://github.com/griefzz/harbour/blob/main/include/harbour/cookies/cookies.hpp) object.
    Then log the information if it exists.

    ```cpp
    auto GetCookies(const Request &req) {
        // When creating a Cookies object from another source
        // we need to handle the possibility of a parsing error
        if (auto cookies = Cookies::create(req)) {
            // The cookies object was parsed successfully
            // Cookies acts as a map<string, string> so grabbing
            // data from the [] operator will automatically insert
            // an entry into the map. If you want to know if a cookie
            // exists in the map use the get method which returns a std::optional
            auto id = cookies.get("id");
            auto name = cookies.get("name");
            if (id && name) // check if the std::optional holds a value
                log::info("id: {}, name: {}", *id, *name);
        }
    }
    ```

## Parameters

While the Cookies object is a simple map, theres quite a bit of metadata associated with cookies
that the browser can accept. Harbour makes this as easy as possible using the Cookies ```with_*``` methods.

!!! example

    This example will construct a cookie with several parameters.

    ```cpp
        // Create an expires for ten days from now
        auto now               = std::chrono::system_clock::now();
        auto ten_days          = std::chrono::hours(24 * 10);
        auto ten_days_from_now = std::chrono::system_clock::to_time_t(now + ten_days);

        // Create a Cookies object with some parameters
        auto cookies = Cookies::create()
                       .with_expires(ten_days_from_now) // Cookie expires 10 days from now
                       .with_http_only()                // Only allow HTTP Responses to set cookies
                       .with_secure();                  // Only allow this cookie on SSL connections
    ```

!!! note

    Heres the full list of a Cookies ```with_*``` parameters

    ```cpp
        // Set the Expires attribute of the cookie to a certain date
        auto with_expires(const std::chrono::system_clock::time_point &date) -> Cookies &;

        // Set the Max-Age attribute of a cookie
        auto with_age(const std::chrono::seconds &age) -> Cookies &;

        // Set the Max-Age attribute of a cookie
        auto with_age(std::size_t age) -> Cookies &;

        // Set the Domain attribute of a cookie
        auto with_domain(auto &&domain) -> Cookies &;

        // Set the Path attribute of a cookie
        auto with_path(auto &&path) -> Cookies &;

        // Set the SameSite attribute of a cookie
        auto with_same_site_policy(cookies::SameSitePolicy policy) -> Cookies &;

        // Enable the Secure flag for a cookie
        auto with_secure() -> Cookies &;

        // Enable the HttpOnly flag for a cookie
        auto with_http_only() -> Cookies &;
    ```

## Converting Cookies

The [Cookies](https://github.com/griefzz/harbour/blob/main/include/harbour/cookies/cookies.hpp) object implements a [fmt::formatter](https://github.com/griefzz/harbour/blob/main/include/harbour/cookies/cookies.hpp#L177).
and the internal [Cookies::Map](https://github.com/griefzz/harbour/blob/main/include/harbour/cookies/map.hpp) implements the JSON [adl_serializer](https://github.com/griefzz/harbour/blob/main/include/harbour/cookies/map.hpp#L28).

You can convert a Cookie::Map to and from json as well as printing out like so

!!! example

    ```cpp
    auto cookies = Cookies::create();
    cookies["id"] = "123";
    cookies["name"] = "bob";

    // Convert the map to json
    auto j = json::serialize(cookies.data);

    // Convert the Cookies object to a string
    auto str = tmpl::render("{}", cookies); // id=123; name=bob
    ```
