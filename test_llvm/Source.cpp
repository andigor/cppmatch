#include <iostream>

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

StatementMatcher mmm = cxxMemberCallExpr( callee( cxxMethodDecl(hasName("LogInfo") ) ) ).bind("mmm");

class LoopPrinter : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult& Result) {

    if (const CXXMemberCallExpr* FS = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("mmm")) {
      FS->dump();
      auto& manager = Result.Context->getSourceManager();
      std::cout << "file_name: " << manager.getFilename( FS->getExprLoc() ).str() << std::endl;
      for (const auto& a : FS->arguments()) {
        const auto& begin_loc = a->getBeginLoc();
        const auto& end_loc   = a->getEndLoc();

        SourceLocation real_end = clang::Lexer::getLocForEndOfToken(end_loc, 0, manager, clang::LangOptions{});

        std::cout << "start_line:"<< manager.getSpellingLineNumber(begin_loc) << std::endl;
        std::cout << "end_line:"  << manager.getSpellingLineNumber(real_end) << std::endl;

        std::cout << "start_col:" << manager.getSpellingColumnNumber(begin_loc) << std::endl;
        std::cout << "end_col:"   << manager.getSpellingColumnNumber(real_end) << std::endl;
      }
    }
  }
};


static llvm::cl::OptionCategory CastMatcherCategory("cast-matcher options");
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static llvm::cl::extrahelp MoreHelp("\nThis tool helps the detection of explicit"
  "C-style casts.");

int main(int argc, const char** argv)
{
  CommonOptionsParser op(argc, argv, CastMatcherCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  // auto ret = Tool.run(newFrontendActionFactory<clang::SyntaxOnlyAction>().get());

  LoopPrinter Printer;
  MatchFinder Finder;
  //Finder.addMatcher(LoopMatcher, &Printer);
  Finder.addMatcher(mmm, &Printer);

  auto ret = Tool.run(newFrontendActionFactory(&Finder).get());

  return ret;
}