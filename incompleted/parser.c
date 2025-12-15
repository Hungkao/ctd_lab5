/* * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Cần thêm để sử dụng strcpy

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "debug.h"
#include "symtab.h" // Cần thiết để sử dụng các hàm quản lý SymTab

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern SymTab* symtab;

void scan(void) {
	Token* tmp = currentToken;
	currentToken = lookAhead;
	lookAhead = getValidToken();
	free(tmp);
}

void eat(TokenType tokenType) {
	if (lookAhead->tokenType == tokenType) {
		scan();
	} else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
	// TODO: create, enter, and exit program block
	Object* program = NULL;
	eat(KW_PROGRAM);
	
	// 1. Tạo Program Object, đặt tên
	if (lookAhead->tokenType == TK_IDENT) {
		program = createProgramObject(lookAhead->string);
	} else {
		// Tạo đối tượng mặc định nếu không có tên
		program = createProgramObject("KW_PROGRAM"); 
	}
	eat(TK_IDENT);

	// 2. Vào scope của program
	enterBlock(program->progAttrs->scope);

	eat(SB_SEMICOLON);
	compileBlock();
	eat(SB_PERIOD);

	// 3. Thoát khỏi scope
	exitBlock();
}

void compileBlock(void) {
	// TODO: create and declare constant objects
	if (lookAhead->tokenType == KW_CONST) {
		eat(KW_CONST);

		do {
			Object* constObj = NULL;
			ConstantValue* constVal = NULL;
			
			// Lấy tên hằng số
			if (lookAhead->tokenType == TK_IDENT) {
				constObj = createConstantObject(lookAhead->string);
			} else {
				// Tạo đối tượng tạm thời nếu không phải IDENT (sẽ báo lỗi ở eat)
				constObj = createConstantObject("");
			}
			eat(TK_IDENT);
			
			// 2. Khai báo (add) vào scope hiện tại
			declareObject(constObj);

			eat(SB_EQ);
			// 3. Phân tích hằng số và gán giá trị
			constVal = compileConstant();
			constObj->constAttrs->value = constVal;

			eat(SB_SEMICOLON);
		} while (lookAhead->tokenType == TK_IDENT);

		compileBlock2();
	} 
	else compileBlock2();
}

void compileBlock2(void) {
	// TODO: create and declare type objects
	if (lookAhead->tokenType == KW_TYPE) {
		eat(KW_TYPE);

		do {
			Object* typeObj = NULL;
			Type* actualType = NULL;

			// Lấy tên kiểu dữ liệu
			if (lookAhead->tokenType == TK_IDENT) {
				typeObj = createTypeObject(lookAhead->string);
			} else {
				typeObj = createTypeObject("");
			}
			eat(TK_IDENT);
			
			// 2. Khai báo (add) vào scope hiện tại
			declareObject(typeObj);

			eat(SB_EQ);
			// 3. Phân tích kiểu dữ liệu và gán actualType
			actualType = compileType();
			typeObj->typeAttrs->actualType = actualType;

			eat(SB_SEMICOLON);
		} while (lookAhead->tokenType == TK_IDENT);

		compileBlock3();
	} 
	else compileBlock3();
}

void compileBlock3(void) {
	// TODO: create and declare variable objects
	if (lookAhead->tokenType == KW_VAR) {
		eat(KW_VAR);

		do {
			Object* varObj = NULL;
			Type* varType = NULL;

			// Lấy tên biến
			if (lookAhead->tokenType == TK_IDENT) {
				varObj = createVariableObject(lookAhead->string);
			} else {
				varObj = createVariableObject("");
			}
			eat(TK_IDENT);
			
			// 2. Khai báo (add) vào scope hiện tại
			declareObject(varObj);

			eat(SB_COLON);
			// 3. Phân tích kiểu dữ liệu và gán type
			varType = compileType();
			varObj->varAttrs->type = varType;

			eat(SB_SEMICOLON);
		} while (lookAhead->tokenType == TK_IDENT);

		compileBlock4();
	} 
	else compileBlock4();
}

void compileBlock4(void) {
	compileSubDecls();
	compileBlock5();
}

void compileBlock5(void) {
	eat(KW_BEGIN);
	compileStatements();
	eat(KW_END);
}

void compileSubDecls(void) {
	while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
		if (lookAhead->tokenType == KW_FUNCTION)
			compileFuncDecl();
		else compileProcDecl();
	}
}

void compileFuncDecl(void) {
	// TODO: create and declare a function object
	Object* funcObj = NULL;
	Type* returnType = NULL;
	
	eat(KW_FUNCTION);
	
	// 1. Tạo Function Object
	if (lookAhead->tokenType == TK_IDENT) {
		funcObj = createFunctionObject(lookAhead->string);
	} else {
		funcObj = createFunctionObject("");
	}
	// 2. Khai báo (add) vào scope cha
	declareObject(funcObj);
	// 3. Vào scope của hàm (function scope)
	enterBlock(funcObj->funcAttrs->scope); 

	eat(TK_IDENT);
	compileParams();
	eat(SB_COLON);
	
	// 4. Phân tích kiểu trả về
	returnType = compileBasicType();
	funcObj->funcAttrs->returnType = returnType;

	eat(SB_SEMICOLON);
	compileBlock();
	eat(SB_SEMICOLON);

	// 5. Thoát scope của hàm
	exitBlock();
}

void compileProcDecl(void) {
	// TODO: create and declare a procedure object
	Object* procObj = NULL;
	eat(KW_PROCEDURE);
	
	// 1. Tạo Procedure Object
	if (lookAhead->tokenType == TK_IDENT) {
		procObj = createProcedureObject(lookAhead->string);
	} else {
		procObj = createProcedureObject("");
	}
	// 2. Khai báo (add) vào scope cha
	declareObject(procObj);
	// 3. Vào scope của thủ tục (procedure scope)
	enterBlock(procObj->procAttrs->scope); 
	
	eat(TK_IDENT);
	compileParams();
	eat(SB_SEMICOLON);
	compileBlock();
	eat(SB_SEMICOLON);

	// 4. Thoát scope của thủ tục
	exitBlock();
}

ConstantValue* compileUnsignedConstant(void) {
	// TODO: create and return an unsigned constant value
	ConstantValue* constValue = NULL;

	switch (lookAhead->tokenType) {
	case TK_NUMBER:
		// Lưu giá trị số
		constValue = makeIntConstant(lookAhead->value); 
		eat(TK_NUMBER);
		break;
	case TK_IDENT:
		// TK_IDENT (Hằng số đã khai báo) - sẽ xử lý semantic sau
		// Tạo giá trị hằng số tạm thời
		constValue = (ConstantValue*)malloc(sizeof(ConstantValue)); 
		constValue->type = TP_INT; 
		constValue->intValue = 0;
		eat(TK_IDENT);
		break;
	case TK_CHAR:
		// Lưu giá trị ký tự
		constValue = makeCharConstant(lookAhead->string[0]);
		eat(TK_CHAR);
		break;
	default:
		error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
		break;
	}
	return constValue;
}

ConstantValue* compileConstant(void) {
	// TODO: create and return a constant
	ConstantValue* constValue = NULL;

	switch (lookAhead->tokenType) {
	case SB_PLUS:
		eat(SB_PLUS);
		constValue = compileConstant2();
		if (constValue != NULL && constValue->type != TP_INT) {
			error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
			free(constValue);
			constValue = NULL;
		}
		break;
	case SB_MINUS:
		eat(SB_MINUS);
		constValue = compileConstant2();
		if (constValue != NULL && constValue->type != TP_INT) {
			error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
			free(constValue);
			constValue = NULL;
		} else if (constValue != NULL) {
			constValue->intValue = -constValue->intValue; // Đảo dấu
		}
		break;
	case TK_CHAR:
		// Lưu giá trị ký tự
		constValue = makeCharConstant(lookAhead->string[0]);
		eat(TK_CHAR);
		break;
	default:
		constValue = compileConstant2();
		break;
	}
	return constValue;
}

ConstantValue* compileConstant2(void) {
	// TODO: create and return a constant value
	ConstantValue* constValue = NULL;

	switch (lookAhead->tokenType) {
	case TK_NUMBER:
		// Lưu giá trị số
		constValue = makeIntConstant(lookAhead->value);
		eat(TK_NUMBER);
		break;
	case TK_IDENT:
		// TK_IDENT (Hằng số đã khai báo) - sẽ xử lý semantic sau
		// Tạo giá trị hằng số tạm thời
		constValue = (ConstantValue*)malloc(sizeof(ConstantValue)); 
		constValue->type = TP_INT; 
		constValue->intValue = 0;
		eat(TK_IDENT);
		break;
	default:
		error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
		break;
	}
	return constValue;
}

Type* compileType(void) {
	// TODO: create and return a type
	Type* type = NULL;

	switch (lookAhead->tokenType) {
	case KW_INTEGER: 
		eat(KW_INTEGER);
		type = intType; // Trả về intType global (chia sẻ)
		break;
	case KW_CHAR: 
		eat(KW_CHAR); 
		type = charType; // Trả về charType global (chia sẻ)
		break;
	case KW_ARRAY:
		eat(KW_ARRAY);
		eat(SB_LSEL);
		
		// Sửa lỗi: Kích thước mảng có thể là TK_NUMBER hoặc TK_IDENT (hằng số)
		if (lookAhead->tokenType == TK_NUMBER || lookAhead->tokenType == TK_IDENT) {
			
			int arraySize = 0;
			if (lookAhead->tokenType == TK_NUMBER) {
				arraySize = lookAhead->value;
				eat(TK_NUMBER);
			} else { // TK_IDENT (sẽ kiểm tra semantic sau)
				eat(TK_IDENT);
			}
			
			eat(SB_RSEL);
			eat(KW_OF);
			
			Type* elementType = compileType();
			type = makeArrayType(arraySize, elementType);

		} else {
			// Lỗi: Kích thước mảng không phải là số hoặc định danh hằng số
			error(ERR_INVALID_ARRAY_SIZE, lookAhead->lineNo, lookAhead->colNo);
			
			// Cơ chế phục hồi lỗi đơn giản: Skip token
			if (lookAhead->tokenType == SB_RSEL) 
				eat(SB_RSEL);
			if (lookAhead->tokenType == KW_OF)
				eat(KW_OF);
			
			// Vẫn cần gọi compileType để tiếp tục phân tích phần còn lại của Type
			Type* elementType = compileType(); 
			type = makeArrayType(0, elementType); // Trả về kiểu lỗi với size 0
		}
		break;
	case TK_IDENT:
		// TK_IDENT (Kiểu dữ liệu đã khai báo) - sẽ xử lý semantic sau
		eat(TK_IDENT);
		type = (Type*)malloc(sizeof(Type)); // Kiểu tạm
		type->typeClass = TP_INT; // Giả định kiểu int tạm thời
		break;
	default:
		error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
		break;
	}
	return type;
}

Type* compileBasicType(void) {
	// TODO: create and return a basic type
	Type* type = NULL;

	switch (lookAhead->tokenType) {
	case KW_INTEGER: 
		eat(KW_INTEGER); 
		type = intType; // Trả về intType global (chia sẻ)
		break;
	case KW_CHAR: 
		eat(KW_CHAR); 
		type = charType; // Trả về charType global (chia sẻ)
		break;
	default:
		error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
		break;
	}
	return type;
}

void compileParams(void) {
	if (lookAhead->tokenType == SB_LPAR) {
		eat(SB_LPAR);
		compileParam();
		while (lookAhead->tokenType == SB_SEMICOLON) {
			eat(SB_SEMICOLON);
			compileParam();
		}
		eat(SB_RPAR);
	}
}

void compileParam(void) {
	// TODO: create and declare a parameter
	Object* paramObj = NULL;
	Type* paramType = NULL;
	enum ParamKind kind = PARAM_VALUE;

	// Lấy owner hiện tại (Function/Procedure)
	Object* owner = symtab->currentScope->owner; 
	
	switch (lookAhead->tokenType) {
	case TK_IDENT:
		// 1. Tạo Parameter Object (value parameter)
		if (lookAhead->tokenType == TK_IDENT) {
			paramObj = createParameterObject(lookAhead->string, PARAM_VALUE, owner);
		} else {
			paramObj = createParameterObject("", PARAM_VALUE, owner);
		}
		eat(TK_IDENT);
		kind = PARAM_VALUE;

		eat(SB_COLON);
		paramType = compileBasicType();
		
		// 2. Gán type
		paramObj->paramAttrs->type = paramType;
		// 3. Khai báo (add) vào scope hiện tại và paramList của owner
		declareObject(paramObj);
		break;
	case KW_VAR:
		eat(KW_VAR);
		// 1. Tạo Parameter Object (variable/reference parameter)
		if (lookAhead->tokenType == TK_IDENT) {
			paramObj = createParameterObject(lookAhead->string, PARAM_REFERENCE, owner);
		} else {
			paramObj = createParameterObject("", PARAM_REFERENCE, owner);
		}
		eat(TK_IDENT);
		kind = PARAM_REFERENCE;

		eat(SB_COLON);
		paramType = compileBasicType();

		// 2. Gán type
		paramObj->paramAttrs->type = paramType;
		// 3. Khai báo (add) vào scope hiện tại và paramList của owner
		declareObject(paramObj);
		break;
	default:
		error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
		break;
	}
}

void compileStatements(void) {
	// Kiểm tra Empty Statement: nếu lookAhead là FOLLOW của Statement (;, END, ELSE)
	// thì không gọi compileStatement()
	if (lookAhead->tokenType == SB_SEMICOLON || lookAhead->tokenType == KW_END || lookAhead->tokenType == KW_ELSE) {
		return;
	}
	
	compileStatement();
	while (lookAhead->tokenType == SB_SEMICOLON) {
		eat(SB_SEMICOLON);
		compileStatement();
	}
}

void compileStatement(void) {
	switch (lookAhead->tokenType) {
	case TK_IDENT:
		compileAssignSt();
		break;
	case KW_CALL:
		compileCallSt();
		break;
	case KW_BEGIN:
		compileGroupSt();
		break;
	case KW_IF:
		compileIfSt();
		break;
	case KW_WHILE:
		compileWhileSt();
		break;
	case KW_FOR:
		compileForSt();
		break;
		// EmptySt
	case SB_SEMICOLON:
	case KW_END:
	case KW_ELSE:
		break;
		// Error occurs
	default:
		error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
		break;
	}
}

void compileLValue(void) {
	eat(TK_IDENT);
	compileIndexes();
}

void compileAssignSt(void) {
	compileLValue();
	eat(SB_ASSIGN);
	compileExpression();
}

void compileCallSt(void) {
	eat(KW_CALL);
	eat(TK_IDENT);
	compileArguments();
}

void compileGroupSt(void) {
	eat(KW_BEGIN);
	compileStatements();
	eat(KW_END);
}

void compileIfSt(void) {
	eat(KW_IF);
	compileCondition();
	eat(KW_THEN);
	compileStatement();
	if (lookAhead->tokenType == KW_ELSE) 
		compileElseSt();
}

void compileElseSt(void) {
	eat(KW_ELSE);
	compileStatement();
}

void compileWhileSt(void) {
	eat(KW_WHILE);
	compileCondition();
	eat(KW_DO);
	compileStatement();
}

void compileForSt(void) {
	eat(KW_FOR);
	eat(TK_IDENT);
	eat(SB_ASSIGN);
	compileExpression();
	eat(KW_TO);
	compileExpression();
	eat(KW_DO);
	compileStatement();
}

void compileArgument(void) {
	compileExpression();
}

void compileArguments(void) {
	switch (lookAhead->tokenType) {
	case SB_LPAR:
		eat(SB_LPAR);
		compileArgument();

		while (lookAhead->tokenType == SB_COMMA) {
			eat(SB_COMMA);
			compileArgument();
		}

		eat(SB_RPAR);
		break;
		// Check FOLLOW set 
	case SB_TIMES:
	case SB_SLASH:
	case SB_PLUS:
	case SB_MINUS:
	case KW_TO:
	case KW_DO:
	case SB_RPAR:
	case SB_COMMA:
	case SB_EQ:
	case SB_NEQ:
	case SB_LE:
	case SB_LT:
	case SB_GE:
	case SB_GT:
	case SB_RSEL:
	case SB_SEMICOLON:
	case KW_END:
	case KW_ELSE:
	case KW_THEN:
		break;
	default:
		error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
	}
}

void compileCondition(void) {
	compileExpression();
	switch (lookAhead->tokenType) {
	case SB_EQ:
		eat(SB_EQ);
		break;
	case SB_NEQ:
		eat(SB_NEQ);
		break;
	case SB_LE:
		eat(SB_LE);
		break;
	case SB_LT:
		eat(SB_LT);
		break;
	case SB_GE:
		eat(SB_GE);
		break;
	case SB_GT:
		eat(SB_GT);
		break;
	default:
		error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
	}

	compileExpression();
}

void compileExpression(void) {
	switch (lookAhead->tokenType) {
	case SB_PLUS:
		eat(SB_PLUS);
		compileExpression2();
		break;
	case SB_MINUS:
		eat(SB_MINUS);
		compileExpression2();
		break;
	default:
		compileExpression2();
	}
}

void compileExpression2(void) {
	compileTerm();
	compileExpression3();
}


void compileExpression3(void) {
	switch (lookAhead->tokenType) {
	case SB_PLUS:
		eat(SB_PLUS);
		compileTerm();
		compileExpression3();
		break;
	case SB_MINUS:
		eat(SB_MINUS);
		compileTerm();
		compileExpression3();
		break;
		// check the FOLLOW set
	case KW_TO:
	case KW_DO:
	case SB_RPAR:
	case SB_COMMA:
	case SB_EQ:
	case SB_NEQ:
	case SB_LE:
	case SB_LT:
	case SB_GE:
	case SB_GT:
	case SB_RSEL:
	case SB_SEMICOLON:
	case KW_END:
	case KW_ELSE:
	case KW_THEN:
		break;
	default:
		error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
	}
}

void compileTerm(void) {
	compileFactor();
	compileTerm2();
}

void compileTerm2(void) {
	switch (lookAhead->tokenType) {
	case SB_TIMES:
		eat(SB_TIMES);
		compileFactor();
		compileTerm2();
		break;
	case SB_SLASH:
		eat(SB_SLASH);
		compileFactor();
		compileTerm2();
		break;
		// check the FOLLOW set
	case SB_PLUS:
	case SB_MINUS:
	case KW_TO:
	case KW_DO:
	case SB_RPAR:
	case SB_COMMA:
	case SB_EQ:
	case SB_NEQ:
	case SB_LE:
	case SB_LT:
	case SB_GE:
	case SB_GT:
	case SB_RSEL:
	case SB_SEMICOLON:
	case KW_END:
	case KW_ELSE:
	case KW_THEN:
		break;
	default:
		error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
	}
}

void compileFactor(void) {
	switch (lookAhead->tokenType) {
	case TK_NUMBER:
		eat(TK_NUMBER);
		break;
	case TK_CHAR:
		eat(TK_CHAR);
		break;
	case TK_IDENT:
		eat(TK_IDENT);
		switch (lookAhead->tokenType) {
		case SB_LPAR:
			compileArguments();
			break;
		case SB_LSEL:
			compileIndexes();
			break;
		default:
			break;
		}
		break;
	default:
		error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
	}
}

void compileIndexes(void) {
	while (lookAhead->tokenType == SB_LSEL) {
		eat(SB_LSEL);
		compileExpression();
		eat(SB_RSEL);
	}
}

int compile(char *fileName) {
	if (openInputStream(fileName) == IO_ERROR)
		return IO_ERROR;

	currentToken = NULL;
	lookAhead = getValidToken();

	initSymTab();

	compileProgram();

	printObject(symtab->program,0);

	cleanSymTab();

	free(currentToken);
	free(lookAhead);
	closeInputStream();
	return IO_SUCCESS;

}