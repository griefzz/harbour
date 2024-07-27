///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///
/// @file blog.cpp
/// @brief Contains the example implementation of a blog with harbour
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <harbour/harbour.hpp>

using namespace harbour;

/// @brief This represents a single blog post
struct Article {
    std::string title;///< Title of blog post
    std::string body; ///< Content body of the post

    /// @brief Convert an Article to an html string
    auto string() const -> std::string {
        return tmpl::render(R"(<div><h2>{}</h2><p>{}</p></div>)", title, body);
    }

    /// @brief Enable the article to be converted to and from json
    HARBOUR_JSONABLE(Article, title, body);
};

/// @brief This represents a collection of our blog posts
struct Blog {
    /// Blog posts are stored in a map with
    /// an incremented ID for each new post
    std::unordered_map<std::size_t, Article> articles;

    /// @brief Convert a Blog into an html string
    auto string() const -> std::string {
        std::string b;
        for (const auto &a: articles)
            b += tmpl::render("<hr>{}", a.second.string());

        return b;
    }
};

/// @brief This is our api endpoints
class Api {
    Harbour &hb;     ///< Harbour instance for managing the API
    Blog &blog;      ///< Blog that can be accessed by each endpoint
    std::mutex mutex;///< Mutex to prevent race conditions

public:
    /// @brief Create a new article
    auto v1_article_create(const Request &) -> Response;

    /// @brief Read an existing article
    auto v1_article_read(const Request &) -> Response;

    /// @brief Update an existing article
    auto v1_article_update(const Request &) -> Response;

    /// @brief Delete an existing article
    auto v1_article_delete(const Request &) -> Response;

    /// @brief Dock our API endpoints
    auto dock() -> void {
        // Since our api end points aren't static functions
        // wrap them in a lambda before passing them into dock
        hb.dock("/api/v1/article/create", [&](const Request &req) { return v1_article_create(req); });
        hb.dock("/api/v1/article/read", [&](const Request &req) { return v1_article_read(req); });
        hb.dock("/api/v1/article/update", [&](const Request &req) { return v1_article_update(req); });
        hb.dock("/api/v1/article/delete", [&](const Request &req) { return v1_article_delete(req); });
    }

    /// @brief Construct an Api with a Blog
    /// @param blog Blog to use
    explicit Api(Harbour &hb, Blog &blog) : hb(hb), blog(blog) {};
};

auto Api::v1_article_create(const Request &req) -> Response {
    // Serialize the requests data into json
    auto data = json::serialize(req.body);

    // Deserialize the json into an Article
    Article article;
    json::deserialize(data, article);

    // Create a lock guard to prevent race conditions
    std::lock_guard<std::mutex> lock(mutex);

    // Create a static id variable to use for our blog post database
    static std::size_t id = 0;

    // Insert the article into our blog database
    blog.articles[id] = article;

    // Increment the id for future blog posts
    id++;

    // Return a response for the newly created post
    return "Created new post!";
}

auto Api::v1_article_read(const Request &req) -> Response {
    // Serialize the requests data body into a json::json_t and get its id value
    auto data = json::serialize(req.body);
    auto id   = data["id"].get<std::size_t>();

    // If our requests id matches a blog post in the database
    if (blog.articles.contains(id)) {
        // Create a lock guard to prevent race conditions
        std::lock_guard<std::mutex> lock(mutex);

        // Return our article as a json response
        return json::json_t(blog.articles[id]);
    }

    // If the id does not exist in the database return a Bad Request status
    return http::Status::BadRequest;
}

auto Api::v1_article_update(const Request &req) -> Response {
    // Serialize the requests data body into a json::json_t and get its id value
    auto data = json::serialize(req.body);
    auto id   = data["id"].get<std::size_t>();

    // Check to see if our database contains the ID
    if (blog.articles.contains(id)) {
        // Deserialize the json into an Article
        Article article;
        json::deserialize(data, article);

        // Create a lock guard to prevent race conditions
        std::lock_guard<std::mutex> lock(mutex);

        // Insert the updated article in our database
        blog.articles[id] = article;

        // Return a status for the newly updated blog post
        return "Blog post updated!";
    }

    // If the id doesnt exist in our database return a Bad Request status
    return http::Status::BadRequest;
}

auto Api::v1_article_delete(const Request &req) -> Response {
    // Serialize the requests data body into a json::json_t and get its id value
    auto data = json::serialize(req.body);
    auto id   = data["id"].get<std::size_t>();

    // Check to see if our database contains the ID
    if (blog.articles.contains(id)) {
        // Create a lock guard to prevent race conditions
        std::lock_guard<std::mutex> lock(mutex);

        // Delete the article from our database
        blog.articles.erase(id);

        // Return a status for the deleted blog post
        return "Blog post deleted!";
    }

    // If the id doesnt exist in our database return a Bad Request status
    return http::Status::BadRequest;
}

/// @brief Serve our create.html file to create new blog posts
auto Create() {
    return tmpl::load_file("create.html").value_or("Couldnt open create.html");
}

/// @brief Serve our update.html file to edit blog posts
auto Update() {
    return tmpl::load_file("update.html").value_or("Couldnt open update.html");
}

/// @brief Serve our delete.html file to delete blog posts
auto Delete() {
    return tmpl::load_file("delete.html").value_or("Couldnt open delete.html");
}

/// @brief Display the home page
struct Home {
    Blog &blog;///< Blog for rendering posts

    /// @brief Render our index.html template with the content of our blog database
    auto operator()() {
        return tmpl::render_file("index.html", blog.string());
    }
};

auto main() -> int {
    // Construct a Harbour instance
    Harbour hb;

    // Construct a new Blog
    Blog blog;

    // Construct a new Api using our Blog and Harbour
    Api api(hb, blog);

    // Dock our ships for showing html
    hb.dock("/", Home{blog});// Pass along our blog to Home for rendering posts
    hb.dock("/create", Create);
    hb.dock("/update", Update);
    hb.dock("/delete", Delete);

    // Dock our api for modifying the Blog
    api.dock();

    // Begin the Harbour server
    hb.sail();

    return 0;
}