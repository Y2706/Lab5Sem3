#include <iostream>
#include <string>
#include <memory>
#include <vector>

class CodeRenderer {
public:
    virtual ~CodeRenderer() = default;
    virtual std::string render(const std::string& code) const = 0;
};

class PlainCodeRenderer : public CodeRenderer {
public:
    std::string render(const std::string& code) const override {
        return "<pre style='font-family: monospace;'>" + code + "</pre>";
    }
};

class SyntaxHighlighter : public CodeRenderer {
protected:
    std::unique_ptr<CodeRenderer> wrapped;
public:
    SyntaxHighlighter(std::unique_ptr<CodeRenderer> wr) : wrapped(std::move(wr)) {}
    std::string render(const std::string& code) const override {
        return wrapped->render(code);
    }
};

class KeywordHighlighter : public SyntaxHighlighter {
private:
    struct KeywordColor {
        std::string keyword;
        std::string color;
    };
    
    std::vector<KeywordColor> keywordsWithColors = {
        {"int", "blue"},
        {"void", "blue"},
        {"class", "purple"},
        {"public", "purple"},
        {"private", "purple"},
        {"return", "darkblue"},
        {"if", "darkorange"},
        {"else", "darkorange"},
        {"for", "darkorange"},
        {"while", "darkorange"},
        {"#include", "red"},
        {"cout", "blue"},
        {"string", "green"}
    };
    
public:
    KeywordHighlighter(std::unique_ptr<CodeRenderer> wr) : SyntaxHighlighter(std::move(wr)) {}
    
    std::string render(const std::string& code) const override {
        std::string highlighted = code;
        
        for (const auto& kw : keywordsWithColors) {
            size_t pos = 0;
            while ((pos = highlighted.find(kw.keyword, pos)) != std::string::npos) {
                if ((pos == 0 || !isalnum(highlighted[pos-1])) && 
                    (pos + kw.keyword.length() == highlighted.length() || !isalnum(highlighted[pos + kw.keyword.length()]))) {
                    
                    std::string replacement = "<span style='color: " + kw.color + ";'>" + kw.keyword + "</span>";
                    highlighted.replace(pos, kw.keyword.length(), replacement);
                    pos += replacement.length();
                } else {
                    pos += kw.keyword.length();
                }
            }
        }
        return wrapped->render(highlighted);
    }
};

class StringHighlighter : public SyntaxHighlighter {
public:
    StringHighlighter(std::unique_ptr<CodeRenderer> wr) : SyntaxHighlighter(std::move(wr)) {}
    
    std::string render(const std::string& code) const override {
        std::string highlighted = code;
        size_t pos = 0;
        while ((pos = highlighted.find('\"', pos)) != std::string::npos) {
            size_t end = highlighted.find('\"', pos + 1);
            if (end == std::string::npos) break;

            std::string original = highlighted.substr(pos, end - pos + 1);
            std::string replacement = "<span style='color: green;'>" + original + "</span>";
            highlighted.replace(pos, original.length(), replacement);
            pos += replacement.length();
        }
        return wrapped->render(highlighted);
    }
};

class CommentHighlighter : public SyntaxHighlighter {
public:
    CommentHighlighter(std::unique_ptr<CodeRenderer> wr) : SyntaxHighlighter(std::move(wr)) {}
    
    std::string render(const std::string& code) const override {
        std::string highlighted = code;
        size_t pos = 0;
        while ((pos = highlighted.find("//", pos)) != std::string::npos) {
            size_t end = highlighted.find('\n', pos);
            if (end == std::string::npos) end = highlighted.length();
            
            std::string comment = highlighted.substr(pos, end - pos);
            std::string replacement = "<span style='color: gray;'>" + comment + "</span>";
            highlighted.replace(pos, comment.length(), replacement);
            pos += replacement.length();
        }
        return wrapped->render(highlighted);
    }
};

void convertToHTML(const std::string& code, const CodeRenderer& renderer) {
    std::string escapedCode = "";
    for (char c : code) {
        if (c == '<') escapedCode += "&lt;";
        else if (c == '>') escapedCode += "&gt;";
        else escapedCode += c;
    }
    std::cout << renderer.render(escapedCode) << std::endl;
}

int main() {
    std::string cppCode = R"(
#include <iostream>
// Пример кода
int main() {
    string message = "Hello, World!";
    return 0;
}
)";

    std::unique_ptr<CodeRenderer> renderer = std::make_unique<PlainCodeRenderer>();
    renderer = std::make_unique<KeywordHighlighter>(std::move(renderer));
    renderer = std::make_unique<StringHighlighter>(std::move(renderer));
    renderer = std::make_unique<CommentHighlighter>(std::move(renderer));

    convertToHTML(cppCode, *renderer);

    return 0;
}
