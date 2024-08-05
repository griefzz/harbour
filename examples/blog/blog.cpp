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
        return tmpl::render("<header><h1>{}</h1></header><p>{}</p>", title, body);
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
        std::string str;
        for (const auto &article: articles)
            str += tmpl::render("<article>{}</article>", article.second.string());

        return str;
    }

    HARBOUR_JSONABLE(Blog, articles);
};

/// @brief Display the home page
struct Home {
    Blog &blog;///< Blog for rendering posts

    /// @brief Render our index.html template with the content of our blog database
    auto operator()() {
        return tmpl::render_file("./html/index.html", blog.string());
    }
};

/// @brief This is our api endpoints
class Api {
    Harbour &hb;     ///< Harbour instance for managing the API
    Blog &blog;      ///< Blog that can be accessed by each endpoint
    std::mutex mutex;///< Mutex to prevent race conditions

    /// @brief Versioned result type for an API Response
    struct v1_Result {
        enum class Code {
            Ok,             // no error
            Error           // some error
        } code;             // result code
        std::string message;// result message

        HARBOUR_JSONABLE(v1_Result, code, message);
    };

    /// @brief Create a new article
    auto v1_article_create(const Request &) -> json::json_t;

    /// @brief Get all existing articles
    auto v1_article_read(const Request &) -> json::json_t;

    /// @brief Update an existing article
    auto v1_article_update(const Request &) -> json::json_t;

    /// @brief Delete an existing article
    auto v1_article_delete(const Request &) -> json::json_t;

public:
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

auto Api::v1_article_create(const Request &req) -> json::json_t {
    // Convenience types
    using Result = v1_Result;
    using Result::Code::Ok;

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
    return Result{Ok, "Created new post"};
}

auto Api::v1_article_read(const Request &req) -> json::json_t {
    // Convert our blog to a JSON Response.
    // This works because all the standard library containers
    // are convertible to json, and we defined Articles and
    // Blog as HARBOUR_JSONABLE
    return blog;
}

auto Api::v1_article_update(const Request &req) -> json::json_t {
    // Convenience types
    using Result = v1_Result;
    using Result::Code::Error;
    using Result::Code::Ok;

    // Serialize the requests data body into a json::json_t
    auto data = json::serialize(req.body);

    // Get the id value, the javascript will send it as a string
    std::size_t id = std::stoull(data["id"].get<std::string>());

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
        return Result{Ok, "Article updated"};
    }

    // If the id doesnt exist in our database return a Bad Request status
    return Result{Error, "Bad Request"};
}

auto Api::v1_article_delete(const Request &req) -> json::json_t {
    // Convenience types
    using Result = v1_Result;
    using Result::Code::Error;
    using Result::Code::Ok;

    // Serialize the requests data body into a json::json_t
    auto data = json::serialize(req.body);

    // Get the id value, the javascript will send it as a string
    std::size_t id = std::stoull(data["id"].get<std::string>());

    // Check to see if our database contains the ID
    if (blog.articles.contains(id)) {
        // Create a lock guard to prevent race conditions
        std::lock_guard<std::mutex> lock(mutex);

        // Delete the article from our database
        blog.articles.erase(id);

        // Return a status for the deleted blog post
        return Result{Ok, "Blog post deleted"};
    }

    // If the id doesnt exist in our database return a Bad Request status
    return Result{Error, "Bad Request"};
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

    // Dock our ships for showing html
    hb.dock("/", Home{blog});// Pass our blog to Home for rendering posts

    // Enable Verbose logging and allow our html files to be found automatically
    hb.dock(middleware::Verbose, middleware::FileServer("./html/"));

    // Dock our api for modifying the Blog
    api.dock();

    // Begin the Harbour server
    hb.sail();

    return 0;
}