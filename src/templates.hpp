#pragma once
#include <string_view>

using namespace std::string_view_literals;

constexpr std::string_view harbour_source_index_body =
        R"(
<!DOCTYPE html>
<html lang=en>
<meta charset=UTF-8>
<meta content="width=device-width,initial-scale=1" name=viewport>
<script src="https://cdn.tailwindcss.com"></script>
<script>
    tailwind.config = {{
        theme: {{
            extend: {{
                colors: {{
                    gitbg: '#010409',
                    gitfg: '#0d1117',
                    muted: '#21262d',
                    fgdefault: '#e6edf3',
                }}
            }}
        }}
    }}
</script>
<title>Harbour Source Code</title>
<style>
    body {{
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "Noto Sans", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji";
        font-size: 14px;
        line-height: 1.5;
        color: #e6edf3;
        -webkit-font-smoothing: antialiased;
        -moz-osx-font-smoothing: grayscale;
    }}

    .item {{
        display: flex;
        position: relative;
    }}

    .item::before {{
        background-color: #30363d;
        width: 1px;
        bottom: 0;
        content: "";
        display: block;
        left: 0;
        position: absolute;
        top: 0;
    }}
</style>

<body class="bg-gitbg p-4 text-white">
    <div class="grid grid-cols-1 sm:grid-cols-3 gap-4">
        <div></div>
        <!-- Source Code -->
        <div class="max-w-lg">
            <div class="border border-muted bg-gitfg p-0 rounded-md shadow-md">
                <div class="flex items-center justify-between mb-1 ml-1 mr-1 p-1"><a
                        href=https://github.com/griefzz/harbour target=_blank><img alt="GitHub Logo" class="h-6 w-6"
                            src=/github-mark-white.svg></a>
                    <h2 class="font-semibold text-2xl">Harbour</h2><a href=/ class="font-semibold text-1xl"><button
                            class="hover:underline hover:text-blue-400">Home</button></a>
                </div>
                {}
            </div>
        </div>
        <!-- Git Log -->
        <div class="inline-block">
            <div class="bg-gitfg text-white p-4 rounded-lg border border-muted w-96">
                <h2 class="text-md font-semibold mb-4">Latest changes</h2>
                <ul>
                    <div id="commits"></div>
                    <div class="flex border-l border-[#30363d]">
                        <li class="ml-4">
                            <a href="https://github.com/griefzz/harbour/commits/main" class="text-[#7d8590] text-xs">
                                View changelog →
                            </a>
                        </li>
                    </div>
                </ul>
            </div>
        </div>
    </div>
</body>

<script>
    async function get_commits() {{
        const response = await fetch("/gitlog");
        return await response.json();
    }}

    get_commits().then((json) => {{
        let commits = document.getElementById("commits");
        json.commits.forEach(j => {{
            commits.innerHTML += `                    <div class="item pt-0 pb-4">
                        <svg aria-hidden="true" height="16" viewBox="0 0 16 16" version="1.1" width="16"
                            data-view-component="true" class="octicon octicon-dot-fill mb-2 fill-[#30363d] ml-[-7px]">
                            <path d="M8 4a4 4 0 1 1 0 8 4 4 0 0 1 0-8Z"></path>
                        </svg>
                        <li class="mb-2 ml-2">
                            <span class="text-[#e5c07b] text-xs">commit ${{j.commit}}</span>
                            <p></p>
                            <span class="text-[#7d8590] text-xs">Author: ${{j.author}}</span>
                            <p></p>
                            <span class="text-[#7d8590] text-xs">Date: ${{j.date}}</span>
                            <p class="mt-4 ml-4">${{j.message}}</p>
                        </li>
                    </div>`;
        }});
    }});

</script>

</html>
)"sv;


constexpr std::string_view harbour_source_index_item =
        R"(
    <div class="bg-gitbg border-t border-muted flex items-center justify-between mb-0 p-2"><a href={}>
                    <p class="text-sm hover:text-blue-400 hover:underline">{}</p></a>
                    <p class="text-sm text-gray-400">{}</p>
                </div>
)"sv;

constexpr std::string_view harbour_source_item =
        R"(
<!DOCTYPE html>
<meta charset=UTF-8>
<title>/harbour/include/harbour/config.hpp</title>
<meta content="width=device-width,initial-scale=1" name=viewport>
<link href=https://cdn.jsdelivr.net/npm/highlight.js@10.7.2/styles/nord.css rel=stylesheet>
<script src=https://unpkg.com/@highlightjs/cdn-assets@11.9.0/highlight.min.js></script>
<script src=//cdnjs.cloudflare.com/ajax/libs/highlightjs-line-numbers.js/2.8.0/highlightjs-line-numbers.min.js></script>
<script>hljs.highlightAll(), hljs.initLineNumbersOnLoad()</script>
<style>
    body,
    html {{
        padding: 0;
        margin: auto;
        font-size: 1.1em;
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "Noto Sans", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji";
        -webkit-font-smoothing: antialiased;
        -moz-osx-font-smoothing: grayscale;
        height: 100%;
        width: 100%;
        background: #0d1117
    }}

    .breadcrumb {{
        background: #010409;
        font-size: .7em;
        color: gray
    }}

    .hljs {{
        background: #0d1117
    }}

    .hljs-ln-numbers {{
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        text-align: center;
        color: #ccc;
        border-right: 1px solid #ccc;
        vertical-align: top;
        padding-right: 5px
    }}

    .hljs-ln-code {{
        padding-left: 10px
    }}
</style>
<div class=breadcrumb><a href=/harbour/>Back</a> - {}
    <hr>
</div>
<pre><code class=language-cpp>{}</code></pre>
</body>
</html>
)";