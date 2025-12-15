/* * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "error.h"

void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode *objList);
void freeReferenceList(ObjectNode *objList);

SymTab* symtab;
Type* intType;
Type* charType;

/******************* Type utilities ******************************/

Type* makeIntType(void) {
    Type* type = (Type*) malloc(sizeof(Type));
    type->typeClass = TP_INT;
    return type;
}

Type* makeCharType(void) {
    Type* type = (Type*) malloc(sizeof(Type));
    type->typeClass = TP_CHAR;
    return type;
}

Type* makeArrayType(int arraySize, Type* elementType) {
    Type* type = (Type*) malloc(sizeof(Type));
    type->typeClass = TP_ARRAY;
    type->arraySize = arraySize;
    type->elementType = elementType;
    return type;
}

Type* duplicateType(Type* type) {
    if (type == NULL) return NULL;
    
    Type* resultType = (Type*) malloc(sizeof(Type));
    resultType->typeClass = type->typeClass;
    if (type->typeClass == TP_ARRAY) {
        resultType->arraySize = type->arraySize;
        resultType->elementType = duplicateType(type->elementType);
    }
    return resultType;
}

int compareType(Type* type1, Type* type2) {
    if (type1 == NULL || type2 == NULL) return 0;
    
    if (type1->typeClass == type2->typeClass) {
        if (type1->typeClass == TP_ARRAY) {
            if (type1->arraySize == type2->arraySize)
                return compareType(type1->elementType, type2->elementType);
            else return 0;
        } else return 1;
    } else return 0;
}

void freeType(Type* type) {
    if (type == NULL) return;
    
    switch (type->typeClass) {
    case TP_INT:
    case TP_CHAR:
        free(type);
        break;
    case TP_ARRAY:
        freeType(type->elementType);
        // Sửa: Chỉ free type, không gọi freeType(type) lần nữa
        free(type); 
        break;
    }
}

/******************* Constant utility ******************************/

ConstantValue* makeIntConstant(int i) {
    ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
    value->type = TP_INT;
    value->intValue = i;
    return value;
}

ConstantValue* makeCharConstant(char ch) {
    ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
    value->type = TP_CHAR;
    value->charValue = ch;
    return value;
}

ConstantValue* duplicateConstantValue(ConstantValue* v) {
    if (v == NULL) return NULL;

    ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
    value->type = v->type;
    if (v->type == TP_INT) 
        value->intValue = v->intValue;
    else
        value->charValue = v->charValue;
    return value;
}

/******************* Object utilities ******************************/

Scope* createScope(Object* owner, Scope* outer) {
    Scope* scope = (Scope*) malloc(sizeof(Scope));
    scope->objList = NULL;
    scope->owner = owner;
    scope->outer = outer;
    return scope;
}

Object* createProgramObject(char *programName) {
    Object* program = (Object*) malloc(sizeof(Object));
    strcpy(program->name, programName);
    program->kind = OBJ_PROGRAM;
    program->progAttrs = (ProgramAttributes*) malloc(sizeof(ProgramAttributes));
    program->progAttrs->scope = createScope(program,NULL);
    symtab->program = program;

    return program;
}

Object* createConstantObject(char *name) {
    Object* obj = (Object*) malloc(sizeof(Object));
    strcpy(obj->name, name);
    obj->kind = OBJ_CONSTANT;
    obj->constAttrs = (ConstantAttributes*) malloc(sizeof(ConstantAttributes));
    obj->constAttrs->value = NULL; // Thêm khởi tạo
    return obj;
}

Object* createTypeObject(char *name) {
    Object* obj = (Object*) malloc(sizeof(Object));
    strcpy(obj->name, name);
    obj->kind = OBJ_TYPE;
    obj->typeAttrs = (TypeAttributes*) malloc(sizeof(TypeAttributes));
    obj->typeAttrs->actualType = NULL; // Thêm khởi tạo
    return obj;
}

Object* createVariableObject(char *name) {
    Object* obj = (Object*) malloc(sizeof(Object));
    strcpy(obj->name, name);
    obj->kind = OBJ_VARIABLE;
    obj->varAttrs = (VariableAttributes*) malloc(sizeof(VariableAttributes));
    obj->varAttrs->type = NULL; // Thêm khởi tạo
    obj->varAttrs->scope = symtab->currentScope;
    return obj;
}

Object* createFunctionObject(char *name) {
    Object* obj = (Object*) malloc(sizeof(Object));
    strcpy(obj->name, name);
    obj->kind = OBJ_FUNCTION;
    obj->funcAttrs = (FunctionAttributes*) malloc(sizeof(FunctionAttributes));
    obj->funcAttrs->returnType = NULL; // Thêm khởi tạo
    obj->funcAttrs->paramList = NULL;
    obj->funcAttrs->scope = createScope(obj, symtab->currentScope);
    return obj;
}

Object* createProcedureObject(char *name) {
    Object* obj = (Object*) malloc(sizeof(Object));
    strcpy(obj->name, name);
    obj->kind = OBJ_PROCEDURE;
    obj->procAttrs = (ProcedureAttributes*) malloc(sizeof(ProcedureAttributes));
    obj->procAttrs->paramList = NULL;
    obj->procAttrs->scope = createScope(obj, symtab->currentScope);
    return obj;
}

Object* createParameterObject(char *name, enum ParamKind kind, Object* owner) {
    Object* obj = (Object*) malloc(sizeof(Object));
    strcpy(obj->name, name);
    obj->kind = OBJ_PARAMETER;
    obj->paramAttrs = (ParameterAttributes*) malloc(sizeof(ParameterAttributes));
    obj->paramAttrs->kind = kind;
    obj->paramAttrs->type = NULL; // Thêm khởi tạo
    obj->paramAttrs->function = owner;
    return obj;
}

void freeObject(Object* obj) {
    if (obj == NULL) return; // Bảo vệ

    switch (obj->kind) {
    case OBJ_CONSTANT:
      if (obj->constAttrs->value != NULL) free(obj->constAttrs->value);
      free(obj->constAttrs);
      break;
    case OBJ_TYPE:
      if (obj->typeAttrs->actualType != NULL) freeType(obj->typeAttrs->actualType);
      free(obj->typeAttrs);
      break;
    case OBJ_VARIABLE:
      // Sửa: KHÔNG free type vì nó có thể là intType/charType hoặc kiểu chia sẻ
      free(obj->varAttrs);
      break;
    case OBJ_FUNCTION:
      // Sửa: Phải dùng freeObjectList để giải phóng các OBJ_PARAMETER và node của chúng
      if (obj->funcAttrs->paramList != NULL) freeObjectList(obj->funcAttrs->paramList);
      if (obj->funcAttrs->returnType != NULL) freeType(obj->funcAttrs->returnType);
      freeScope(obj->funcAttrs->scope);
      free(obj->funcAttrs);
      break;
    case OBJ_PROCEDURE:
      // Sửa: Phải dùng freeObjectList để giải phóng các OBJ_PARAMETER và node của chúng
      if (obj->procAttrs->paramList != NULL) freeObjectList(obj->procAttrs->paramList);
      freeScope(obj->procAttrs->scope);
      free(obj->procAttrs);
      break;
    case OBJ_PROGRAM:
      freeScope(obj->progAttrs->scope);
      free(obj->progAttrs);
      break;
    case OBJ_PARAMETER:
      // Sửa: KHÔNG free type
      free(obj->paramAttrs);
      break;
    }
    free(obj);
}

void freeScope(Scope* scope) {
    if (scope == NULL) return; // Bảo vệ

    // freeObjectList sẽ giải phóng các đối tượng trong scope
    freeObjectList(scope->objList);
    free(scope);
}

void freeObjectList(ObjectNode *objList) {
    ObjectNode* list = objList;

    while (list != NULL) {
      ObjectNode* node = list;
      list = list->next;
      
      // >>> KHẮC PHỤC DOUBLE FREE THAM SỐ (QUAN TRỌNG) <<<
      // CHỈ giải phóng Object nếu nó KHÔNG phải là Tham số
      if (node->object->kind != OBJ_PARAMETER) {
          freeObject(node->object);
      }
      
      free(node);
    }
}

void freeReferenceList(ObjectNode *objList) {
    ObjectNode* list = objList;

    while (list != NULL) {
      ObjectNode* node = list;
      list = list->next;
      // Hàm này chỉ giải phóng node, không giải phóng object
      free(node);
    }
}

void addObject(ObjectNode **objList, Object* obj) {
    ObjectNode* node = (ObjectNode*) malloc(sizeof(ObjectNode));
    node->object = obj;
    node->next = NULL;
    if ((*objList) == NULL) 
        *objList = node;
    else {
        ObjectNode *n = *objList;
        while (n->next != NULL) 
            n = n->next;
        n->next = node;
    }
}

Object* findObject(ObjectNode *objList, char *name) {
    while (objList != NULL) {
        if (strcmp(objList->object->name, name) == 0) 
            return objList->object;
        else objList = objList->next;
    }
    return NULL;
}

/******************* others ******************************/

void initSymTab(void) {
    Object* obj;
    Object* param;

    symtab = (SymTab*) malloc(sizeof(SymTab));
    symtab->globalObjectList = NULL;
    
    // Khởi tạo các hàm/thủ tục built-in
    
    obj = createFunctionObject("READC");
    obj->funcAttrs->returnType = makeCharType();
    addObject(&(symtab->globalObjectList), obj);

    obj = createFunctionObject("READI");
    obj->funcAttrs->returnType = makeIntType();
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITEI");
    param = createParameterObject("i", PARAM_VALUE, obj);
    param->paramAttrs->type = makeIntType();
    addObject(&(obj->procAttrs->paramList),param);
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITEC");
    param = createParameterObject("ch", PARAM_VALUE, obj);
    param->paramAttrs->type = makeCharType();
    addObject(&(obj->procAttrs->paramList),param);
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITELN");
    addObject(&(symtab->globalObjectList), obj);

    // Khởi tạo kiểu dữ liệu cơ sở toàn cục
    intType = makeIntType();
    charType = makeCharType();
}

void cleanSymTab(void) {
    // Giải phóng Program Object
    if (symtab->program != NULL) freeObject(symtab->program);
    
    // Giải phóng Built-in Objects
    if (symtab->globalObjectList != NULL) freeObjectList(symtab->globalObjectList);
    
    free(symtab);
    
    // Giải phóng kiểu dữ liệu cơ sở toàn cục
    freeType(intType);
    freeType(charType);
}

void enterBlock(Scope* scope) {
    symtab->currentScope = scope;
}

void exitBlock(void) {
    symtab->currentScope = symtab->currentScope->outer;
}

Object* lookupObject(char *name) {
    // TODO: Hoàn thành hàm lookupObject
    Scope* scope = symtab->currentScope;
    Object* obj;
    
    // 1. Tìm kiếm từ scope hiện tại đi ngược lên scope cha
    while (scope != NULL) {
        obj = findObject(scope->objList, name);
        if (obj != NULL) return obj;
        scope = scope->outer;
    }
    
    // 2. Tìm kiếm trong danh sách đối tượng toàn cục (built-in objects)
    return findObject(symtab->globalObjectList, name);
}

void declareObject(Object* obj) {
    if (obj->kind == OBJ_PARAMETER) {
        Object* owner = symtab->currentScope->owner;
        switch (owner->kind) {
        case OBJ_FUNCTION:
            addObject(&(owner->funcAttrs->paramList), obj);
            break;
        case OBJ_PROCEDURE:
            addObject(&(owner->procAttrs->paramList), obj);
            break;
        default:
            break;
        }
    }
    
    addObject(&(symtab->currentScope->objList), obj);
}