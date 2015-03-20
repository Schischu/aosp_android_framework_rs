/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <strings.h>

#include "Generator.h"
#include "Scanner.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

// API level when RenderScript was added.
const int MIN_API_LEVEL = 9;

const NumericalType TYPES[] = {
            {"f16", "FLOAT_16", "half", "half", FLOATING_POINT, 11, 5},
            {"f32", "FLOAT_32", "float", "float", FLOATING_POINT, 24, 8},
            {"f64", "FLOAT_64", "double", "double", FLOATING_POINT, 53, 11},
            {"i8", "SIGNED_8", "char", "byte", SIGNED_INTEGER, 7, 0},
            {"u8", "UNSIGNED_8", "uchar", "byte", UNSIGNED_INTEGER, 8, 0},
            {"i16", "SIGNED_16", "short", "short", SIGNED_INTEGER, 15, 0},
            {"u16", "UNSIGNED_16", "ushort", "short", UNSIGNED_INTEGER, 16, 0},
            {"i32", "SIGNED_32", "int", "int", SIGNED_INTEGER, 31, 0},
            {"u32", "UNSIGNED_32", "uint", "int", UNSIGNED_INTEGER, 32, 0},
            {"i64", "SIGNED_64", "long", "long", SIGNED_INTEGER, 63, 0},
            {"u64", "UNSIGNED_64", "ulong", "long", UNSIGNED_INTEGER, 64, 0},
};

const int NUM_TYPES = sizeof(TYPES) / sizeof(TYPES[0]);

const char BASE_URL[] = "http://developer.android.com/reference/android/graphics/drawable/";

// The singleton of the collected information of all the spec files.
SystemSpecification systemSpecification;

// Returns the index in TYPES for the provided cType
static int findCType(const string& cType) {
    for (int i = 0; i < NUM_TYPES; i++) {
        if (cType == TYPES[i].cType) {
            return i;
        }
    }
    return -1;
}

/* Converts a string like "u8, u16" to a vector of "ushort", "uint".
 * For non-numerical types, we don't need to convert the abbreviation.
 */
static vector<string> convertToTypeVector(const string& input) {
    // First convert the string to an array of strings.
    vector<string> entries;
    stringstream stream(input);
    string entry;
    while (getline(stream, entry, ',')) {
        trimSpaces(&entry);
        entries.push_back(entry);
    }

    /* Second, we look for present numerical types. We do it this way
     * so the order of numerical types is always the same, no matter
     * how specified in the spec file.
     */
    vector<string> result;
    for (auto t : TYPES) {
        for (auto i = entries.begin(); i != entries.end(); ++i) {
            if (*i == t.specType) {
                result.push_back(t.cType);
                entries.erase(i);
                break;
            }
        }
    }

    // Add the remaining; they are not numerical types.
    for (auto s : entries) {
        result.push_back(s);
    }

    return result;
}

void ParameterDefinition::parseParameterDefinition(const string& type, const string& name,
                                                   const string& testOption, int lineNumber,
                                                   bool isReturn, Scanner* scanner) {
    rsType = type;
    specName = name;

    // Determine if this is an output.
    isOutParameter = isReturn || charRemoved('*', &rsType);

    // Extract the vector size out of the type.
    int last = rsType.size() - 1;
    char lastChar = rsType[last];
    if (lastChar >= '0' && lastChar <= '9') {
        rsBaseType = rsType.substr(0, last);
        mVectorSize = lastChar;
    } else {
        rsBaseType = rsType;
        mVectorSize = "1";
    }
    if (mVectorSize == "3") {
        vectorWidth = "4";
    } else {
        vectorWidth = mVectorSize;
    }

    /* Create variable names to be used in the java and .rs files.  Because x and
     * y are reserved in .rs files, we prefix variable names with "in" or "out".
     */
    if (isOutParameter) {
        variableName = "out";
        if (!specName.empty()) {
            variableName += capitalize(specName);
        } else if (!isReturn) {
            scanner->error(lineNumber) << "Should have a name.\n";
        }
    } else {
        variableName = "in";
        if (specName.empty()) {
            scanner->error(lineNumber) << "Should have a name.\n";
        }
        variableName += capitalize(specName);
    }
    rsAllocName = "gAlloc" + capitalize(variableName);
    javaAllocName = variableName;
    javaArrayName = "array" + capitalize(javaAllocName);

    // Process the option.
    undefinedIfOutIsNan = false;
    compatibleTypeIndex = -1;
    if (!testOption.empty()) {
        if (testOption.compare(0, 6, "range(") == 0) {
            size_t pComma = testOption.find(',');
            size_t pParen = testOption.find(')');
            if (pComma == string::npos || pParen == string::npos) {
                scanner->error(lineNumber) << "Incorrect range " << testOption << "\n";
            } else {
                minValue = testOption.substr(6, pComma - 6);
                maxValue = testOption.substr(pComma + 1, pParen - pComma - 1);
            }
        } else if (testOption.compare(0, 6, "above(") == 0) {
            size_t pParen = testOption.find(')');
            if (pParen == string::npos) {
                scanner->error(lineNumber) << "Incorrect testOption " << testOption << "\n";
            } else {
                smallerParameter = testOption.substr(6, pParen - 6);
            }
        } else if (testOption.compare(0, 11, "compatible(") == 0) {
            size_t pParen = testOption.find(')');
            if (pParen == string::npos) {
                scanner->error(lineNumber) << "Incorrect testOption " << testOption << "\n";
            } else {
                compatibleTypeIndex = findCType(testOption.substr(11, pParen - 11));
            }
        } else if (testOption.compare(0, 11, "conditional") == 0) {
            undefinedIfOutIsNan = true;
        } else {
            scanner->error(lineNumber) << "Unrecognized testOption " << testOption << "\n";
        }
    }

    typeIndex = findCType(rsBaseType);
    isFloatType = false;
    if (typeIndex >= 0) {
        javaBaseType = TYPES[typeIndex].javaType;
        specType = TYPES[typeIndex].specType;
        isFloatType = TYPES[typeIndex].exponentBits > 0;
    }
    if (!minValue.empty()) {
        if (typeIndex < 0 || TYPES[typeIndex].kind != FLOATING_POINT) {
            scanner->error(lineNumber) << "range(,) is only supported for floating point\n";
        }
    }
}

void VersionInfo::scan(Scanner* scanner) {
    if (scanner->findOptionalTag("version:")) {
        const string s = scanner->getValue();
        sscanf(s.c_str(), "%i %i", &minVersion, &maxVersion);
        if (minVersion && minVersion < MIN_API_LEVEL) {
            scanner->error() << "Minimum version must >= 9\n";
        }
        if (minVersion == MIN_API_LEVEL) {
            minVersion = 0;
        }
        if (maxVersion && maxVersion < MIN_API_LEVEL) {
            scanner->error() << "Maximum version must >= 9\n";
        }
    }
    if (scanner->findOptionalTag("size:")) {
        sscanf(scanner->getValue().c_str(), "%i", &intSize);
    }
}

Definition::Definition(const std::string& name, SpecFile* specFile) : mName(name), mHidden(false) {
    mSpecFileName = specFile->getSpecFileName();
    mUrl = specFile->getDetailedDocumentationUrl() + "#android_rs:" + name;
}

void Definition::scanDocumentationTags(Scanner* scanner, bool firstOccurence) {
    if (scanner->findOptionalTag("hidden:")) {
        scanner->checkNoValue();
        mHidden = true;
    }
    if (firstOccurence) {
        if (scanner->findTag("summary:")) {
            mSummary = scanner->getValue();
        }
        if (scanner->findTag("description:")) {
            scanner->checkNoValue();
            while (scanner->findOptionalTag("")) {
                mDescription.push_back(scanner->getValue());
            }
        }
    } else if (scanner->findOptionalTag("summary:")) {
        scanner->error() << "Only the first specification should have a summary.\n";
    }
}

Constant::~Constant() {
    for (auto i : mSpecifications) {
        delete i;
    }
}

Type::~Type() {
    for (auto i : mSpecifications) {
        delete i;
    }
}

Function::Function(const string& name, SpecFile* specFile) : Definition(name, specFile) {
    mCapitalizedName = capitalize(mName);
}

Function::~Function() {
    for (auto i : mSpecifications) {
        delete i;
    }
}

bool Function::someParametersAreDocumented() const {
    for (auto p : mParameters) {
        if (!p->documentation.empty()) {
            return true;
        }
    }
    return false;
}

void Function::addParameter(ParameterEntry* entry, Scanner* scanner) {
    for (auto i : mParameters) {
        if (i->name == entry->name) {
            // It's a duplicate.
            if (!entry->documentation.empty()) {
                scanner->error(entry->lineNumber)
                            << "Only the first occurence of an arg should have the "
                               "documentation.\n";
            }
            return;
        }
    }
    mParameters.push_back(entry);
}

void Function::addReturn(ParameterEntry* entry, Scanner* scanner) {
    if (entry->documentation.empty()) {
        return;
    }
    if (!mReturnDocumentation.empty()) {
        scanner->error() << "ret: should be documented only for the first variant\n";
    }
    mReturnDocumentation = entry->documentation;
}

void Specification::scanVersionInfo(Scanner* scanner) {
    mVersionInfo.scan(scanner);
}

void ConstantSpecification::scanConstantSpecification(Scanner* scanner, SpecFile* specFile) {
    string name = scanner->getValue();

    bool created = false;
    Constant* constant = specFile->findOrCreateConstant(name, &created);

    ConstantSpecification* spec = new ConstantSpecification();
    constant->addSpecification(spec);

    spec->scanVersionInfo(scanner);
    if (scanner->findTag("value:")) {
        spec->mValue = scanner->getValue();
    }
    constant->scanDocumentationTags(scanner, created);

    scanner->findTag("end:");
}

void TypeSpecification::scanTypeSpecification(Scanner* scanner, SpecFile* specFile) {
    string name = scanner->getValue();

    bool created = false;
    Type* type = specFile->findOrCreateType(name, &created);

    TypeSpecification* spec = new TypeSpecification();
    type->addSpecification(spec);

    spec->scanVersionInfo(scanner);
    if (scanner->findOptionalTag("simple:")) {
        spec->mKind = SIMPLE;
        spec->mSimpleType = scanner->getValue();
    }
    if (scanner->findOptionalTag("struct:")) {
        spec->mKind = STRUCT;
        spec->mStructName = scanner->getValue();
        while (scanner->findOptionalTag("field:")) {
            string s = scanner->getValue();
            string comment;
            scanner->parseDocumentation(&s, &comment);
            spec->mFields.push_back(s);
            spec->mFieldComments.push_back(comment);
        }
        if (scanner->findOptionalTag("attrib:")) {
            spec->mAttrib = scanner->getValue();
        }
    }
    if (scanner->findOptionalTag("enum:")) {
        spec->mKind = ENUM;
        spec->mEnumName = scanner->getValue();
        while (scanner->findOptionalTag("value:")) {
            string s = scanner->getValue();
            string comment;
            scanner->parseDocumentation(&s, &comment);
            spec->mValues.push_back(s);
            spec->mValueComments.push_back(comment);
        }
    }
    type->scanDocumentationTags(scanner, created);

    scanner->findTag("end:");
}

FunctionSpecification::~FunctionSpecification() {
    for (auto i : mParameters) {
        delete i;
    }
    delete mReturn;
    for (auto i : mPermutations) {
        delete i;
    }
}

string FunctionSpecification::expandString(string s, int indexOfReplaceable1,
                                           int indexOfReplaceable2, int indexOfReplaceable3,
                                           int indexOfReplaceable4) const {
    if (mReplaceables.size() > 0) {
        s = stringReplace(s, "#1", mReplaceables[0][indexOfReplaceable1]);
    }
    if (mReplaceables.size() > 1) {
        s = stringReplace(s, "#2", mReplaceables[1][indexOfReplaceable2]);
    }
    if (mReplaceables.size() > 2) {
        s = stringReplace(s, "#3", mReplaceables[2][indexOfReplaceable3]);
    }
    if (mReplaceables.size() > 3) {
        s = stringReplace(s, "#4", mReplaceables[3][indexOfReplaceable4]);
    }
    return s;
}

void FunctionSpecification::expandStringVector(const vector<string>& in, int indexOfReplaceable1,
                                               int indexOfReplaceable2, int indexOfReplaceable3,
                                               int indexOfReplaceable4, vector<string>* out) const {
    out->clear();
    for (vector<string>::const_iterator iter = in.begin(); iter != in.end(); iter++) {
        out->push_back(expandString(*iter, indexOfReplaceable1, indexOfReplaceable2,
                                    indexOfReplaceable3, indexOfReplaceable4));
    }
}

void FunctionSpecification::createPermutations(Function* function, Scanner* scanner) {
    int start[4];
    int end[4];
    for (int i = 0; i < 4; i++) {
        if (i < (int)mReplaceables.size()) {
            start[i] = 0;
            end[i] = mReplaceables[i].size();
        } else {
            start[i] = -1;
            end[i] = 0;
        }
    }
    for (int i4 = start[3]; i4 < end[3]; i4++) {
        for (int i3 = start[2]; i3 < end[2]; i3++) {
            for (int i2 = start[1]; i2 < end[1]; i2++) {
                for (int i1 = start[0]; i1 < end[0]; i1++) {
                    auto p = new FunctionPermutation(function, this, i1, i2, i3, i4, scanner);
                    mPermutations.push_back(p);
                }
            }
        }
    }
}

string FunctionSpecification::getName(int i1, int i2, int i3, int i4) const {
    return expandString(mUnexpandedName, i1, i2, i3, i4);
}

void FunctionSpecification::getReturn(int i1, int i2, int i3, int i4, std::string* retType,
                                      int* lineNumber) const {
    *retType = expandString(mReturn->type, i1, i2, i3, i4);
    *lineNumber = mReturn->lineNumber;
}

void FunctionSpecification::getParam(size_t index, int i1, int i2, int i3, int i4,
                                     std::string* type, std::string* name, std::string* testOption,
                                     int* lineNumber) const {
    ParameterEntry* p = mParameters[index];
    *type = expandString(p->type, i1, i2, i3, i4);
    *name = p->name;
    *testOption = expandString(p->testOption, i1, i2, i3, i4);
    *lineNumber = p->lineNumber;
}

void FunctionSpecification::getInlines(int i1, int i2, int i3, int i4,
                                       std::vector<std::string>* inlines) const {
    expandStringVector(mInline, i1, i2, i3, i4, inlines);
}

void FunctionSpecification::parseTest(Scanner* scanner) {
    const string value = scanner->getValue();
    if (value == "scalar" || value == "vector" || value == "noverify" || value == "custom" ||
        value == "none") {
        mTest = value;
    } else if (value.compare(0, 7, "limited") == 0) {
        mTest = "limited";
        if (value.compare(7, 1, "(") == 0) {
            size_t pParen = value.find(')');
            if (pParen == string::npos) {
                scanner->error() << "Incorrect test: \"" << value << "\"\n";
            } else {
                mPrecisionLimit = value.substr(8, pParen - 8);
            }
        }
    } else {
        scanner->error() << "Unrecognized test option: \"" << value << "\"\n";
    }
}

bool FunctionSpecification::hasTests(int versionOfTestFiles) const {
    if (mVersionInfo.minVersion != 0 && mVersionInfo.minVersion > versionOfTestFiles) {
        return false;
    }
    if (mVersionInfo.maxVersion != 0 && mVersionInfo.maxVersion < versionOfTestFiles) {
        return false;
    }
    if (mTest == "none") {
        return false;
    }
    return true;
}

void FunctionSpecification::scanFunctionSpecification(Scanner* scanner, SpecFile* specFile) {
    // Some functions like convert have # part of the name.  Truncate at that point.
    string name = scanner->getValue();
    size_t p = name.find('#');
    if (p != string::npos) {
        if (p > 0 && name[p - 1] == '_') {
            p--;
        }
        name.erase(p);
    }

    bool created = false;
    Function* function = specFile->findOrCreateFunction(name, &created);

    FunctionSpecification* spec = new FunctionSpecification();
    function->addSpecification(spec);

    spec->mUnexpandedName = scanner->getValue();
    spec->mTest = "scalar";  // default

    spec->scanVersionInfo(scanner);

    if (scanner->findOptionalTag("attrib:")) {
        spec->mAttribute = scanner->getValue();
    }
    if (scanner->findOptionalTag("w:")) {
        vector<string> t;
        if (scanner->getValue().find("1") != string::npos) {
            t.push_back("");
        }
        if (scanner->getValue().find("2") != string::npos) {
            t.push_back("2");
        }
        if (scanner->getValue().find("3") != string::npos) {
            t.push_back("3");
        }
        if (scanner->getValue().find("4") != string::npos) {
            t.push_back("4");
        }
        spec->mReplaceables.push_back(t);
    }

    while (scanner->findOptionalTag("t:")) {
        spec->mReplaceables.push_back(convertToTypeVector(scanner->getValue()));
    }

    if (scanner->findTag("ret:")) {
        ParameterEntry* p = scanner->parseArgString(true);
        function->addReturn(p, scanner);
        spec->mReturn = p;
    }
    while (scanner->findOptionalTag("arg:")) {
        ParameterEntry* p = scanner->parseArgString(false);
        function->addParameter(p, scanner);
        spec->mParameters.push_back(p);
    }

    function->scanDocumentationTags(scanner, created);

    if (scanner->findOptionalTag("inline:")) {
        scanner->checkNoValue();
        while (scanner->findOptionalTag("")) {
            spec->mInline.push_back(scanner->getValue());
        }
    }
    if (scanner->findOptionalTag("test:")) {
        spec->parseTest(scanner);
    }

    scanner->findTag("end:");

    spec->createPermutations(function, scanner);
}

FunctionPermutation::FunctionPermutation(Function* func, FunctionSpecification* spec, int i1,
                                         int i2, int i3, int i4, Scanner* scanner)
    : mFunction(func), mReturn(nullptr), mInputCount(0), mOutputCount(0) {
    // We expand the strings now to make capitalization easier.  The previous code preserved
    // the #n
    // markers just before emitting, which made capitalization difficult.
    mName = spec->getName(i1, i2, i3, i4);
    mNameTrunk = func->getName();
    mTest = spec->getTest();
    mPrecisionLimit = spec->getPrecisionLimit();
    spec->getInlines(i1, i2, i3, i4, &mInline);

    mHasFloatAnswers = false;
    for (size_t i = 0; i < spec->getNumberOfParams(); i++) {
        string type, name, testOption;
        int lineNumber = 0;
        spec->getParam(i, i1, i2, i3, i4, &type, &name, &testOption, &lineNumber);
        ParameterDefinition* def = new ParameterDefinition();
        def->parseParameterDefinition(type, name, testOption, lineNumber, false, scanner);
        if (def->isOutParameter) {
            mOutputCount++;
        } else {
            mInputCount++;
        }

        if (def->typeIndex < 0 && mTest != "none") {
            scanner->error(lineNumber)
                        << "Could not find " << def->rsBaseType
                        << " while generating automated tests.  Use test: none if not needed.\n";
        }
        if (def->isOutParameter && def->isFloatType) {
            mHasFloatAnswers = true;
        }
        mParams.push_back(def);
    }

    string retType;
    int lineNumber = 0;
    spec->getReturn(i1, i2, i3, i4, &retType, &lineNumber);
    if (!retType.empty()) {
        mReturn = new ParameterDefinition();
        mReturn->parseParameterDefinition(retType, "", "", lineNumber, true, scanner);
        if (mReturn->isFloatType) {
            mHasFloatAnswers = true;
        }
        mOutputCount++;
    }
}

FunctionPermutation::~FunctionPermutation() {
    for (auto i : mParams) {
        delete i;
    }
    delete mReturn;
}

SpecFile::SpecFile(const string& specFileName) : mSpecFileName(specFileName) {
    string core = mSpecFileName;
    // Remove .spec
    size_t l = core.length();
    const char SPEC[] = ".spec";
    const int SPEC_SIZE = sizeof(SPEC) - 1;
    const int start = l - SPEC_SIZE;
    if (start >= 0 && core.compare(start, SPEC_SIZE, SPEC) == 0) {
        core.erase(start);
    }

    // The header file name should have the same base but with a ".rsh" extension.
    mHeaderFileName = core + ".rsh";

    mDetailedDocumentationUrl = BASE_URL;
    mDetailedDocumentationUrl += core + ".html";
}

SpecFile::~SpecFile() {
    for (auto i : mConstantsList) {
        delete i;
    }
    for (auto i : mTypesList) {
        delete i;
    }
    for (auto i : mFunctionsList) {
        delete i;
    }
}

// Read the specification, adding the definitions to the global functions map.
bool SpecFile::readSpecFile() {
    FILE* specFile = fopen(mSpecFileName.c_str(), "rt");
    if (!specFile) {
        cerr << "Error opening input file: " << mSpecFileName << "\n";
        return false;
    }

    Scanner scanner(mSpecFileName, specFile);

    // Scan the header that should start the file.
    scanner.skipBlankEntries();
    if (scanner.findTag("header:")) {
        if (scanner.findTag("summary:")) {
            mBriefDescription = scanner.getValue();
        }
        if (scanner.findTag("description:")) {
            scanner.checkNoValue();
            while (scanner.findOptionalTag("")) {
                mFullDescription.push_back(scanner.getValue());
            }
        }
        if (scanner.findOptionalTag("include:")) {
            scanner.checkNoValue();
            while (scanner.findOptionalTag("")) {
                mVerbatimInclude.push_back(scanner.getValue());
            }
        }
        scanner.findTag("end:");
    }

    while (1) {
        scanner.skipBlankEntries();
        if (scanner.atEnd()) {
            break;
        }
        const string tag = scanner.getNextTag();
        if (tag == "function:") {
            FunctionSpecification::scanFunctionSpecification(&scanner, this);
        } else if (tag == "type:") {
            TypeSpecification::scanTypeSpecification(&scanner, this);
        } else if (tag == "constant:") {
            ConstantSpecification::scanConstantSpecification(&scanner, this);
        } else {
            scanner.error() << "Expected function:, type:, or constant:.  Found: " << tag << "\n";
            return false;
        }
    }

    fclose(specFile);
    return scanner.getErrorCount() == 0;
}

// Returns the named entry in the map.  Creates it if it's not there.
template <class T>
T* findOrCreate(const string& name, list<T*>* list, map<string, T*>* map, bool* created,
                SpecFile* specFile) {
    auto iter = map->find(name);
    if (iter != map->end()) {
        *created = false;
        return iter->second;
    }
    *created = true;
    T* f = new T(name, specFile);
    map->insert(pair<string, T*>(name, f));
    list->push_back(f);
    return f;
}

Constant* SpecFile::findOrCreateConstant(const string& name, bool* created) {
    return findOrCreate<Constant>(name, &mConstantsList, &mConstantsMap, created, this);
}

Type* SpecFile::findOrCreateType(const string& name, bool* created) {
    return findOrCreate<Type>(name, &mTypesList, &mTypesMap, created, this);
}

Function* SpecFile::findOrCreateFunction(const string& name, bool* created) {
    return findOrCreate<Function>(name, &mFunctionsList, &mFunctionsMap, created, this);
}

SystemSpecification::~SystemSpecification() {
    for (auto i : mSpecFiles) {
        delete i;
    }
}

template <class T>
static bool addDefinitionToMap(map<string, T*>* map, T* object) {
    const string& name = object->getName();
    auto i = map->find(name);
    if (i != map->end()) {
        T* existing = i->second;
        cerr << object->getSpecFileName() << ": Error. " << name << " has already been defined in "
             << existing->getSpecFileName() << "\n";
        return false;
    }
    (*map)[name] = object;
    return true;
}

bool SystemSpecification::readSpecFile(const string& fileName) {
    SpecFile* spec = new SpecFile(fileName);
    if (!spec->readSpecFile()) {
        cerr << fileName << ": Failed to parse.\n";
        return false;
    }
    mSpecFiles.push_back(spec);

    // Store links to the definitions in a global table.
    bool success = true;
    for (auto i : spec->getConstantsMap()) {
        if (!addDefinitionToMap(&mConstants, i.second)) {
            success = false;
        }
    }
    for (auto i : spec->getTypesMap()) {
        if (!addDefinitionToMap(&mTypes, i.second)) {
            success = false;
        }
    }
    for (auto i : spec->getFunctionsMap()) {
        if (!addDefinitionToMap(&mFunctions, i.second)) {
            success = false;
        }
    }

    return success;
}

bool SystemSpecification::generateFiles(int versionOfTestFiles) const {
    bool success = GenerateHeaderFiles() && generateHtmlDocumentation() &&
                   GenerateTestFiles(versionOfTestFiles);
    if (success) {
        cout << "Successfully processed " << mTypes.size() << " types, " << mConstants.size()
             << " constants, and " << mFunctions.size() << " functions.\n";
    }
    return success;
}

string SystemSpecification::getHtmlAnchor(const string& name) const {
    Definition* d = nullptr;
    auto c = mConstants.find(name);
    if (c != mConstants.end()) {
        d = c->second;
    } else {
        auto t = mTypes.find(name);
        if (t != mTypes.end()) {
            d = t->second;
        } else {
            auto f = mFunctions.find(name);
            if (f != mFunctions.end()) {
                d = f->second;
            } else {
                return string();
            }
        }
    }
    ostringstream stream;
    stream << "<a href='" << d->getUrl() << "'>" << name << "</a>";
    return stream.str();
}
