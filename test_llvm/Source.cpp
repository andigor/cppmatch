

//#include <iostream>
//#include <clang-c/Index.h>  // This is libclang.
//
//std::ostream& operator<<(std::ostream& stream, const CXString& str)
//{
//  stream << clang_getCString(str);
//  clang_disposeString(str);
//  return stream;
//}
//
//int main()
//{
//  CXIndex index = clang_createIndex(0, 0);
//
//  CXTranslationUnit unit = clang_parseTranslationUnit(
//    index,
//    "C:/Source/src/eXLerate/Source/xlConnect/BlockSock.cpp", nullptr, 0,
//    nullptr, 0,
//    CXTranslationUnit_None);
//
//  if (unit == nullptr)
//  {
//    std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
//    return -1;
//  }
//
//  CXCursor cursor = clang_getTranslationUnitCursor(unit);
//  clang_visitChildren(
//    cursor,
//    [](CXCursor c, CXCursor parent, CXClientData client_data)
//    {
//      std::cout << "Cursor '" << clang_getCursorSpelling(c) << "' of kind '"
//        << clang_getCursorKindSpelling(clang_getCursorKind(c)) << "'\n";
//      return CXChildVisit_Recurse;
//    },
//    nullptr);
//
//  clang_disposeIndex(index);
//
//  //C:/Source/src/eXLerate/Source/xlConnect/BlockSock.cpp
//
//
//
//  return 0;
//}


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

//StatementMatcher LoopMatcher =
//forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(
//  hasInitializer(integerLiteral(equals(0)))))))).bind("forLoop");

//StatementMatcher mmm = cxxMemberCallExpr( callee( cxxMethodDecl(hasName("GetNextAssoc") ) ) ).bind("mmm"); !! works
StatementMatcher mmm = cxxMemberCallExpr( callee( cxxMethodDecl(hasName("LogInfo") ) ) ).bind("mmm");
//StatementMatcher mmm = implicitCastExpr(hasImplicitDestinationType( asString("ATL::CStringT<char, StrTraitMFC_DLL<char> >") ) ).bind("mmm");
//StatementMatcher mmm = implicitCastExpr( hasSourceExpression( ) ).bind("mmm");
//StatementMatcher mmm = cxxBindTemporaryExpr( cxxConstructExpr( hasDeclaration( cxxDestructorDecl( hasName( "CStringT" ) ) ) ) ).bind("mmm");
//StatementMatcher mmm = cxxBindTemporaryExpr( has( cxxConstructExpr( hasDeclaration( cxxConstructorDecl( hasName( "CStringT" ) ) ) ) ) ).bind("mmm"); // !! works

//StatementMatcher mmm = cxxBindTemporaryExpr( has( cxxConstructExpr( hasDeclaration( cxxConstructorDecl( hasName( "CStringT" ) ) ) ) ), hasDescendant(memberExpr().bind("member"))).bind("mmm"); // !! works

//StatementMatcher mmm = cxxBindTemporaryExpr( has(
//                                                  cxxConstructExpr( hasDeclaration( cxxConstructorDecl( hasName( "CStringT" ) ) ) )
//                                                )
//                                           ).bind("mmm"); // !! works


class LoopPrinter : public MatchFinder::MatchCallback {
public :
  virtual void run(const MatchFinder::MatchResult& Result) {
    //if (const ForStmt *FS = Result.Nodes.getNodeAs<clang::ForStmt>("forLoop"))
    if (const CXXMemberCallExpr* FS = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("mmm")) {
      FS->dump();
      auto& manager = Result.Context->getSourceManager();
      std::cout << "file_name: " << manager.getFilename( FS->getExprLoc() ).str() << std::endl;
      for (const auto& a : FS->arguments()) {
        const auto& begin_loc = a->getBeginLoc();
        const auto& end_loc = a->getEndLoc();

        std::cout << "start_line:"<< manager.getSpellingLineNumber(begin_loc) << std::endl;
        std::cout << "end_line:"  << manager.getSpellingLineNumber(end_loc) << std::endl;

        std::cout << "start_col:" << manager.getSpellingColumnNumber(begin_loc) << std::endl;
        std::cout << "end_col:"   << manager.getSpellingColumnNumber(end_loc) << std::endl;
      }
    }

    //if (const CXXBindTemporaryExpr* FS = Result.Nodes.getNodeAs<clang::CXXBindTemporaryExpr>("mmm")) {
    //  auto& manager = Result.Context->getSourceManager();
    //
    //  std::cout << "file_name: " << manager.getFilename(FS->getBeginLoc()).str() << std::endl;
    //  std::cout << "start_line:"<< manager.getSpellingLineNumber(FS->getBeginLoc()) << std::endl;
    //  std::cout << "end_line:"  << manager.getSpellingLineNumber(FS->getEndLoc()) << std::endl;
    //
    //  std::cout << "start_col:" << manager.getSpellingColumnNumber(FS->getBeginLoc()) << std::endl;
    //  std::cout << "end_col:"   << manager.getSpellingColumnNumber(FS->getEndLoc()) << std::endl;
    //
    //  FS->dump();
    //
    //  //std::cout << FS->getMethodDecl()->getNameAsString() << std::endl;
    //  // std::cout << FS->getCallee()->getType()->getAsCXXRecordDecl()->getName().str() << std::endl;
    //  //std::cout << FS->getDirectCallee()->getNameAsString() << std::endl;
    //}
    //
    //if (const MemberExpr* FS = Result.Nodes.getNodeAs<clang::MemberExpr>("member")) {
    //  auto& manager = Result.Context->getSourceManager();
    //
    //  std::cout << "2file_name: " << manager.getFilename(FS->getBeginLoc()).str() << std::endl;
    //  std::cout << "2start_line:"<< manager.getSpellingLineNumber(FS->getBeginLoc()) << std::endl;
    //  std::cout << "2end_line:"  << manager.getSpellingLineNumber(FS->getEndLoc()) << std::endl;
    //
    //  std::cout << "2start_col:" << manager.getSpellingColumnNumber(FS->getBeginLoc()) << std::endl;
    //  std::cout << "2end_col:"   << manager.getSpellingColumnNumber(FS->getEndLoc()) << std::endl;
    //
    //  std::cout << FS->getMemberNameInfo().getAsString() << std::endl;
    //
    //  FS->dump();
    //
    //  //std::cout << FS->getMethodDecl()->getNameAsString() << std::endl;
    //  // std::cout << FS->getCallee()->getType()->getAsCXXRecordDecl()->getName().str() << std::endl;
    //  //std::cout << FS->getDirectCallee()->getNameAsString() << std::endl;
    //}

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