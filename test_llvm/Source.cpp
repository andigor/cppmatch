#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unordered_map>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;


#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang;
using namespace clang::ast_matchers;

auto prevent_double_get_string =
  unless(
    hasAncestor(
      memberExpr(
        hasDeclaration(
          decl(
            cxxMethodDecl(
              hasName("GetString")
            )
          )
        )
      )
    )
  );


StatementMatcher cstring_cast_matcher = cxxMemberCallExpr(
                                               anyOf(
                                                  callee( cxxMethodDecl(hasName("LogInfo" ) ) )
                                                , callee( cxxMethodDecl(hasName("LogDebug") ) )
                                                , callee( cxxMethodDecl(hasName("LogError") ) )
                                               )
                                               , allOf (
                                                   forEach(
                                                     cxxBindTemporaryExpr(
                                                       hasType(
                                                         asString("CString")
                                                       )
                                                     ).bind( "temp_arg" )
                                                   )
                                                   ,
                                                   forEach(
                                                     conditionalOperator(
                                                       hasType(
                                                         asString("CString")
                                                       )
                                                     ).bind( "conditional_arg" )
                                                   )
                                                 , anything()
                                                 )
                                         ).bind("cstring_cast_matcher");

struct file_line {
  std::string line_;
  ssize_t offset_ = 0;
};

class file_content {
public:
  //file_content()
  //{
  //
  //}
  file_content(const std::string& file_name)
  {
    load(file_name);
  }

  void save_to(const std::string& file_name)
  {
    std::fstream f;
    f.open(file_name, std::fstream::out);

    if (!f.is_open()) {
      throw std::runtime_error("cannot open file for writing: " + file_name);
    }
    for (const auto& s : lines_) {
      f << s.line_ << '\n';
    }

    f.close();
  }
  void insert_text(size_t row, size_t col, const char* txt)
  {
    file_line& line = lines_.at(row);
    size_t len = std::strlen(txt);
    line.line_.insert(line.offset_ + col, txt);
    line.offset_ += len;
  }
private:
  void load(const std::string& file_name)
  {
    std::fstream f;

    f.open(file_name, std::fstream::in);

    if ( !f.is_open() ) {
      throw std::runtime_error("cannot open file: " + file_name);
    }

    read_lines(f);

    f.close();
  }

  void read_lines(std::fstream& f)
  {
    std::string line;
    lines_.clear();

    for (std::string line; std::getline(f, line, '\n'); ) {
      lines_.push_back(file_line{line, 0});
    }
  }

  std::vector<file_line> lines_;
};



class files_content {
public:
  files_content()
  {
  }

  file_content& get_file_data(const std::string& file_name)
  {
    auto iter = files_.find(file_name);
    if (iter == files_.end()) {
      auto ret = files_.insert(std::make_pair(file_name, file_content{ file_name }));
      iter = ret.first;
    }
    return iter->second;
  }

  void save_all()
  {
    for (auto& info : files_) {
      info.second.save_to(info.first);
    }
  }
private:
  std::unordered_map<std::string, file_content> files_;
};

class matcher_callback : public MatchFinder::MatchCallback {
private:
  files_content& files_content_;
public:
  matcher_callback(files_content& content)
    :files_content_(content)
  {

  }
  virtual void run(const MatchFinder::MatchResult& Result) {
    if (const Expr* a = Result.Nodes.getNodeAs<clang::Expr>("temp_arg")) {
      insert_explicit_get_string(a, Result.Context);
    }

    if (const ConditionalOperator* a = Result.Nodes.getNodeAs<clang::ConditionalOperator>("conditional_arg")) {
      a->dump();
      insert_explicit_static_cast(a, Result.Context);
    }

//    if (const CXXMemberCallExpr* FS = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("cstring_cast_matcher")) {
//      //auto& manager = Result.Context;
//
//      //auto file_name = manager.getFilename(FS->getExprLoc()).str();
//      //FS->dump();
//      for (const auto& a : FS->arguments()) {
//        if (a->getType().getAsString() != "CString") {
//          continue;
//        }
//
////        if (auto op = dyn_cast_or_null<clang::ConditionalOperator>(a)) {
////          op->dump();
////
////          // assuming no recursive operators here
////          if (op->getLHS()->getType().getAsString() == "CString") {
////            //insert_explicit_get_string(file_name, op->getLHS(), manager);
////            //std::cout << "LHS!" << std::endl;
////          }
////
////          if (op->getRHS()->getType().getAsString() == "CString") {
////            //insert_explicit_get_string(file_name, op->getRHS(), manager);
////            //std::cout << "RHS!" << std::endl;
////          }
////
////        }
//        else {
//          //insert_explicit_get_string(file_name, a, manager);
//          insert_explicit_static_cast(a,  Result.Context);
//        }
//      }
//    }
//
    //if ( const MemberExpr* a = Result.Nodes.getNodeAs<clang::MemberExpr>("nested_member") ) {
    //  //std::cout << "haha: " << a->getType().getAsString() << std::endl;
    //  if (a->getType().getAsString() == "CString") {
    //    //a->dump();
    //
    //    auto& manager = Result.Context->getSourceManager();
    //    auto file_name = manager.getFilename(a->getExprLoc()).str();
    //    insert_explicit_get_string(file_name, a, manager);
    //  }
    //}

    //if ( const CXXMemberCallExpr* a = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("nested_call") ) {
    //  //std::cout << "haha: " << a->getType().getAsString() << std::endl;
    //  if (a->getType().getAsString() == "CString") {
    //    //a->dump();
    //
    //    auto& manager = Result.Context->getSourceManager();
    //    auto file_name = manager.getFilename(a->getExprLoc()).str();
    //    insert_explicit_get_string(file_name, a, manager);
    //  }
    //}
  }

  template <class Node, class Manager>
  SourceLocation get_real_end(Node& node, Manager& manager) const
  {
    const auto& end_loc = node->getEndLoc();
    SourceLocation real_end = clang::Lexer::getLocForEndOfToken(end_loc, 0, manager, clang::LangOptions{});
    return real_end;
  }

  template <class Node>
  void insert_explicit_get_string(Node n, const clang::ASTContext* context) {
    const auto& manager = context->getSourceManager();
    auto file_name = manager.getFilename(n->getExprLoc()).str();

    auto real_end = get_real_end( n, manager );

    const unsigned line_num = manager.getSpellingLineNumber(real_end);
    const unsigned col_num = manager.getSpellingColumnNumber(real_end);

    file_content& content = files_content_.get_file_data(file_name);
    content.insert_text(line_num - 1, col_num - 1, ".GetString( )");
  }

  template <class Node>
  void insert_explicit_static_cast(Node n, const clang::ASTContext* context) {
    const auto& manager = context->getSourceManager();
    auto file_name = manager.getFilename(n->getExprLoc()).str();
    file_content& content = files_content_.get_file_data(file_name);

    {
      const auto& start_loc = n->getBeginLoc();

      const auto full_loc = context->getFullLoc(start_loc);

      //full_loc.dump();



      const unsigned start_line_num = manager.getSpellingLineNumber(start_loc);
      const unsigned start_col_num = manager.getSpellingColumnNumber(start_loc);

      content.insert_text(start_line_num - 1, start_col_num - 1, "static_cast<const char*>( ");
    }
    {
      auto real_end = get_real_end( n, manager );

      const unsigned end_line_num = manager.getSpellingLineNumber(real_end);
      const unsigned end_col_num = manager.getSpellingColumnNumber(real_end);

      content.insert_text(end_line_num - 1, end_col_num - 1, " )");
    }
  }
};


static llvm::cl::OptionCategory CastMatcherCategory("warnings remover");
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static llvm::cl::extrahelp MoreHelp("\n More help needed");

int main(int argc, const char** argv)
{
  CommonOptionsParser op(argc, argv, CastMatcherCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  files_content content;

  try {
    {
      matcher_callback Printer(content);
      MatchFinder Finder;

      Finder.addMatcher(cstring_cast_matcher, &Printer);

      Tool.run(newFrontendActionFactory(&Finder).get());
    }

    content.save_all();
  }
  catch (const std::runtime_error & err) {
    std::cerr << "exception: " << err.what() << std::endl;
  }
  catch (...) {
    std::cerr << "unknown exception: " << std::endl;
  }


  return 0;
}