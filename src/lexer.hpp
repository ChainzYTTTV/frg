#pragma once
#include "common.hpp"

#include <string>
#include <vector>

enum class Tok {
    Ident, IntLit, FloatLit, StrLit, RawStrLit, Hash,
    LParen, RParen, LBrace, RBrace, LBracket, RBracket, Comma, Semi, Dot, Colon, Question,
    Plus, Minus, Star, Slash, Percent, PlusPlus, MinusMinus,
    Assign, PlusEq, MinusEq, StarEq, SlashEq, PercentEq,
    DotDot, EqEq, NotEq, Lt, Gt, LtEq, GtEq, AndAnd, OrOr, Bang, Amp, Pipe, Caret, Tilde, Shl, Shr, QuestionQuestion,
    KwIf, KwElse, KwWhile, KwDo, KwInterface, KwAbstract, KwTry, KwCatch, KwThrow, KwFor, KwReturn, KwBreak, KwContinue, KwTrue, KwFalse, KwNull,
    KwAnd, KwOr, KwNot, KwConst, KwVoid,
    KwStruct, KwClass, KwEnum, KwSwitch, KwCase, KwDefault, KwThis, KwSuper, KwIn, KwStatic, KwIs, KwAs, KwPrivate, KwPublic, KwNegative, KwBitNot,
    KwReserved,  // words saved for future versions (new, delete)
    Eof,
};

struct Token {
    Tok kind;
    std::string text;  // for StrLit: the raw characters between the quotes
    Loc loc;
};

std::vector<Token> lexSource(const SourceFile& file);
// Lexes the code inside a string's {interpolation}, which starts at (line, col) in `file`.
std::vector<Token> lexFragment(const SourceFile& file, const std::string& text, int line, int col);
bool isKeyword(const std::string& word);
