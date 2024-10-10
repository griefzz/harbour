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
    std::string title;
    std::string body;

    /// @brief Convert an Article to an html string
    auto string() const -> std::string {
        return tmpl::render("<header><h1>{}</h1></header><p>{}</p>", title, body);
    }
};

/// @brief This represents a collection of our blog posts
struct Blog {
    /// Blog posts are stored in a map with
    /// an incremented ID for each new post
    std::unordered_map<std::size_t, Article> articles;

    /// @brief Convert a Blog into an html string
    auto string() const -> std::string {
        std::string str;
        for (const auto &article: articles)
            str += tmpl::render("<article>{}</article>", article.second.string());

        return str;
    }
};

/// @brief Display the home page
struct Home {
    Blog &blog;

    /// @brief Render our index.html template with the content of our blog database
    auto operator()() {
        return tmpl::render_file("./html/index.html", blog.string())
                .value_or("Unable to find index.html");
    }
};

/// @brief This class represents a collection of our api endpoints
class Api {
    Harbour &hb;
    Blog &blog;
    std::mutex mutex;///< Mutex to prevent race conditions

    /// @brief Result type for an API Response
    struct Result {
        enum class Code {
            Ok,
            Error
        } code;
        std::string message;
    };

    /// @brief Create a new article
    auto article_create(const Request &) -> json;

    /// @brief Get all existing articles
    auto article_read(const Request &) -> json;

    /// @brief Update an existing article
    auto article_update(const Request &) -> json;

    /// @brief Delete an existing article
    auto article_delete(const Request &) -> json;

public:
    /// @brief Dock our API endpoints
    auto dock() -> void {
        // Since our api end points aren't static functions
        // wrap them in a lambda before passing them into dock
        hb.dock("/api/article/create", [&](const Request &req) { return article_create(req); });
        hb.dock("/api/article/read", [&](const Request &req) { return article_read(req); });
        hb.dock("/api/article/update", [&](const Request &req) { return article_update(req); });
        hb.dock("/api/article/delete", [&](const Request &req) { return article_delete(req); });
    }

    /// @brief Construct an Api with a Blog
    /// @param blog Blog to use
    explicit Api(Harbour &hb, Blog &blog) : hb(hb), blog(blog) {};
};

auto Api::article_create(const Request &req) -> json {
    // Deserialize the json into an Article
    const auto article = deserialize<Article>(req.body);

    // Create a lock guard to prevent race conditions
    std::lock_guard<std::mutex> lock(mutex);

    // Create a static id variable to use for our blog post database
    static std::size_t id = 0;

    // Insert the article into our blog database
    blog.articles[id] = article;

    // Increment the id for future blog posts
    id++;

    // Return a response for the newly created post
    return Result{Result::Code::Ok, "Created new post"};
}

auto Api::article_read(const Request &req) -> json {
    // Convert our blog to a JSON Response.
    // This works because the Blog type is directly
    // convertible to a json string using reflect-cpp
    return blog;
}

auto Api::article_update(const Request &req) -> json {
    // Expected api request
    struct ApiRequest {
        Article article; // New article data
        std::size_t id; // ID of the article to modify
    };

    // Deserialize the json into our api request type
    auto data = deserialize<ApiRequest>(req.body);

    // Check to see if our database contains the ID
    if (blog.articles.contains(data.id)) {
        // Create a lock guard to prevent race conditions
        std::lock_guard<std::mutex> lock(mutex);

        // Insert the updated article in our database
        blog.articles[data.id] = data.article;

        // Return a status for the newly updated blog post
        return Result{Result::Code::Ok, "Article updated"};
    }

    // If the id doesnt exist in our database return a Bad Request status
    return Result{Result::Code::Error, "Bad Request"};
}

auto Api::article_delete(const Request &req) -> json {
    // Expected api request
    struct ApiRequest {
        std::size_t id;
    };

    // Deserialize the json into our api request type
    auto data = deserialize<ApiRequest>(req.body);

    // Check to see if our database contains the ID
    if (blog.articles.contains(data.id)) {
        // Create a lock guard to prevent race conditions
        std::lock_guard<std::mutex> lock(mutex);

        // Delete the article from our database
        blog.articles.erase(data.id);

        // Return a status for the deleted blog post
        return Result{Result::Code::Ok, "Blog post deleted"};
    }

    // If the id doesnt exist in our database return a Bad Request status
    return Result{Result::Code::Error, "Bad Request"};
}

auto main() -> int {
    // Disable the non-verbose connection callback
    auto settings = server::Settings().with_on_connection(nullptr);

    // Construct a Harbour instance
    Harbour hb(settings);

    // Construct a new Blog
    Blog blog;

    // Construct a new Api using our Blog and Harbour
    Api api(hb, blog);

    // Enable Verbose logging
    hb.dock(middleware::Verbose);

    // Dock our ships for showing html
    hb.dock("/", Home{blog});// Pass our blog to Home for rendering posts

    // Allow our html files to be found automatically and require a password to use admin panel
    hb.dock("/admin/:file", Middleware(middleware::BasicAuth("admin", "password"),
                                       middleware::FileServer("./html/")));

    // Dock our api for modifying the Blog
    api.dock();

    // Begin the Harbour server
    hb.sail();

    return 0;
}