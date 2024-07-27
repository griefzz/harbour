# JSON

Harbour loves JSON and tries to make it as easy as possible to work with.

The included and fantastic [nlohmann::json](https://github.com/nlohmann/json) library provides everything you could want!

Harbour provides a wrapper around the [nlohmann::json](https://github.com/nlohmann/json) type under [json::json_t](https://github.com/griefzz/harbour/blob/main/include/harbour/json.hpp#L20)

## Serialization

Any type that fulfils the [nlohman::json](https://github.com/nlohmann/json?tab=readme-ov-file#examples) type can be used with Harbour's [json::serialize](https://github.com/griefzz/harbour/blob/main/include/harbour/json.hpp#L30)

Harbour provides a convenient method for serialization, but ultimately you're free to use the full power of [nlohmann::json](https://github.com/nlohmann/json) knowing harbour supports and encourages it!

!!! example

    ```cpp
    // Converting raw JSON literal into a json object
    auto pirate = json::serialize(R"({ "ship": "Ol Skippy", "captain": "Pegleg Billy" })");

    // Converting a Cookie::Map into JSON
    auto cookies = json::serialize(cookies.data);

    // Converting a Request body into JSON and echo it back as JSON
    auto ToFromJson(const Request &req) -> json::json_t {
        return json::serialize(req.body);
    }
    ```

## Deserialization

Any type that fulfils the requirements of the [nlohman::json](https://github.com/nlohmann/json?tab=readme-ov-file#examples) type can be used with Harbour's [json::deserialize](https://github.com/griefzz/harbour/blob/main/include/harbour/json.hpp#L35) library can be deserialized!

!!! example

    This example deserializes a pirate json object into a Boat struct 

    ```cpp
    struct Boat {
        std::string ship;
        std::vector<std::string> crew;
        std::string captain;

        // HARBOUR_JSONABLE is a wrapper for NLOHMANN_DEFINE_TYPE_INTRUSIVE
        HARBOUR_JSONABLE(Boat, ship, crew, captain);
    };

    // Create a json object
    auto pirate    = json::serialize(R"({ "ship": "Ol Skippy", "captain": "Pegleg Billy" })"); 
    pirate["crew"] = {"Bobby", "Johhny", "Tommy"}; // Add some pirates

    Boat boat; // Initialize the boat for deserialization
    json::deserialize(pirates, boat); // deserialize pirates into the boat
    ```
