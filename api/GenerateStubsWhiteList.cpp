/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <iostream>
#include <sstream>

#include "Generator.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

#if 0
static void writeConstantComment(GeneratedFile* file, const Constant& constant) {
    const string name = constant.getName();
    writeComment(file, name, constant.getSummary(), constant.getDescription(),
                 constant.deprecated(), true);
}

static void writeConstantSpecification(GeneratedFile* file, const ConstantSpecification& spec) {
    VersionInfo info = spec.getVersionInfo();
    writeVersionGuardStart(file, info);
    *file << "#define " << spec.getConstant()->getName() << " " << spec.getValue() << "\n\n";
    writeVersionGuardEnd(file, info);
}

static void writeTypeSpecification(GeneratedFile* file, const TypeSpecification& spec) {
    const string& typeName = spec.getType()->getName();
    const VersionInfo info = spec.getVersionInfo();
    writeVersionGuardStart(file, info);
    switch (spec.getKind()) {
        case SIMPLE:
            *file << "typedef " << spec.getSimpleType() << " " << typeName << ";\n";
            break;
        case ENUM: {
            *file << "typedef enum ";
            const string name = spec.getEnumName();
            if (!name.empty()) {
                *file << name << " ";
            }
            *file << "{\n";

            const vector<string>& values = spec.getValues();
            const vector<string>& valueComments = spec.getValueComments();
            const size_t last = values.size() - 1;
            for (size_t i = 0; i <= last; i++) {
                *file << "    " << values[i];
                if (i != last) {
                    *file << ",";
                }
                if (valueComments.size() > i && !valueComments[i].empty()) {
                    *file << " // " << valueComments[i];
                }
                *file << "\n";
            }
            *file << "} " << typeName << ";\n";
            break;
        }
        case STRUCT: {
            *file << "typedef struct ";
            const string name = spec.getStructName();
            if (!name.empty()) {
                *file << name << " ";
            }
            *file << "{\n";

            const vector<string>& fields = spec.getFields();
            const vector<string>& fieldComments = spec.getFieldComments();
            for (size_t i = 0; i < fields.size(); i++) {
                *file << "    " << fields[i] << ";";
                if (fieldComments.size() > i && !fieldComments[i].empty()) {
                    *file << " // " << fieldComments[i];
                }
                *file << "\n";
            }
            *file << "} ";
            const string attrib = spec.getAttrib();
            if (!attrib.empty()) {
                *file << attrib << " ";
            }
            *file << typeName << ";\n";
            break;
        }
    }
    writeVersionGuardEnd(file, info);
    *file << "\n";
}

static void writeTypeComment(GeneratedFile* file, const Type& type) {
    const string name = type.getName();
    writeComment(file, name, type.getSummary(), type.getDescription(), type.deprecated(), true);
}

static void writeFunctionPermutation(GeneratedFile* file, const FunctionSpecification& spec,
                                     const FunctionPermutation& permutation) {
    writeVersionGuardStart(file, spec.getVersionInfo());

    // Write linkage info.
    const auto inlineCodeLines = permutation.getInline();
    if (inlineCodeLines.size() > 0) {
        *file << "static inline ";
    } else {
        *file << "extern ";
    }

    // Write the return type.
    auto ret = permutation.getReturn();
    if (ret) {
        *file << ret->rsType;
    } else {
        *file << "void";
    }

    // Write the attribute.
    *file << " __attribute__((";
    const string attrib = spec.getAttribute();
    if (attrib.empty()) {
        *file << "overloadable";
    } else if (attrib[0] == '=') {
        /* If starts with an equal, we don't automatically add overloadable.
         * This is because of the error we made defining rsUnpackColor8888().
         */
        *file << attrib.substr(1);
    } else {
        *file << attrib << ", overloadable";
    }
    *file << "))\n";

    // Write the function name.
    *file << "    " << permutation.getName() << "(";
    const int offset = 4 + permutation.getName().size() + 1;  // Size of above

    // Write the arguments.  We wrap on mulitple lines if a line gets too long.
    int charsOnLine = offset;
    bool hasGenerated = false;
    for (auto p : permutation.getParams()) {
        if (hasGenerated) {
            *file << ",";
            charsOnLine++;
        }
        ostringstream ps;
        ps << p->rsType;
        if (p->isOutParameter) {
            ps << "*";
        }
        if (!p->specName.empty()) {
            ps << " " << p->specName;
        }
        const string s = ps.str();
        if (charsOnLine + s.size() >= 100) {
            *file << "\n" << string(offset, ' ');
            charsOnLine = offset;
        } else if (hasGenerated) {
            *file << " ";
            charsOnLine++;
        }
        *file << s;
        charsOnLine += s.size();
        hasGenerated = true;
    }
    // In C, if no parameters, we need to output void, e.g. fn(void).
    if (!hasGenerated) {
        *file << "void";
    }
    *file << ")";

    // Write the inline code, if any.
    if (inlineCodeLines.size() > 0) {
        *file << " {\n";
        for (size_t ct = 0; ct < inlineCodeLines.size(); ct++) {
            if (inlineCodeLines[ct].empty()) {
                *file << "\n";
            } else {
                *file << "    " << inlineCodeLines[ct] << "\n";
            }
        }
        *file << "}\n";
    } else {
        *file << ";\n";
    }

    writeVersionGuardEnd(file, spec.getVersionInfo());
    *file << "\n";
}

static void writeFunctionComment(GeneratedFile* file, const Function& function) {
    // Write the generic documentation.
    writeComment(file, function.getName(), function.getSummary(), function.getDescription(),
                 function.deprecated(), false);

    // Comment the parameters.
    if (function.someParametersAreDocumented()) {
        *file << " *\n";
        *file << " * Parameters:\n";
        for (auto p : function.getParameters()) {
            if (!p->documentation.empty()) {
                *file << " *   " << p->name << ": " << p->documentation << "\n";
            }
        }
    }

    // Comment the return type.
    const string returnDoc = function.getReturnDocumentation();
    if (!returnDoc.empty()) {
        *file << " *\n";
        *file << " * Returns: " << returnDoc << "\n";
    }

    *file << " */\n";
}

#endif

// If prefix starts input, copy the equivalence to stream and remove it from input.
static void skipPrefix(ostream* stream, string* input, const string& prefix, char equivalent) {
    size_t size = prefix.size();
    if (input->compare(0, size, prefix) != 0) {
        return;
    }
    input->erase(0, size);
    *stream << equivalent;
}

struct AbbreviationEntry {
    const char* type;
    char equivalence;
};

AbbreviationEntry abbreviations[] = {
            {"void", 'v'},
            {"wchar_t", 'w'},
            {"bool", 'b'},
            {"char", 'c'},
            {"signed char", 'a'},
            {"uchar", 'h'},
            {"short", 's'},
            {"ushort", 't'},
            {"int", 'i'},
            {"uint", 'j'},
            {"uint32_t", 'j'},
            {"long", 'l'},
            {"ulong", 'm'},
            {"long long", 'x'},
            {"unsigned long long", 'y'},
            {"float", 'f'},
            {"double", 'd'},
};

static string translateParameter(const ParameterDefinition& p) {
    ostringstream stream;
    string type = p.rsType;
    skipPrefix(&stream, &type, "const ", 'K');

    if (p.mVectorSize != "1") {
        type.erase(type.size() - 1, 1);
        stream << "PKDv" << p.mVectorSize << "_";
    }

    for (auto e : abbreviations) {
        if (type == e.type) {
            stream << e.equivalence;
            return stream.str();
        }
    }

    if (p.typeIndex >= 0) {
        cerr << "Yikes " << type << "\n";
    }
    stream << type.size() << type;
    return stream.str();
    /*
    if (p.typeIndex < 0) {
        *file << p.rsType.size() << p.rsType;
    } else {
        *file << TYPES[p.typeIndex].cSignatureType;
    }
    */
}

static void writeParameters(ostringstream* stream,
                            const std::vector<ParameterDefinition*>& params) {
    if (params.empty()) {
        *stream << "v";
    } else {
        vector<string> previous;
        for (ParameterDefinition* p : params) {
            if (p->isOutParameter) {
                *stream << "P";
            }
            const string coding = translateParameter(*p);
            if (coding.size() <= 1) {
                *stream << coding;
            } else {
                bool found = false;
                for (size_t i = 0; i < previous.size(); ++i) {
                    if (previous[i] == coding) {
                        *stream << 'S';
                        if (i > 0) {
                            *stream << (char)('0' + i - 1);
                        }
                        *stream << '_';
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    *stream << coding;
                    previous.push_back(coding);
                }
            }
        }
    }
}

static void writePrototype(set<string>* entries, const Function& function,
                           const FunctionPermutation& permutation, bool overloadable,
                           bool vectorImplementedInCore) {
    auto params = permutation.getParams();
    if (vectorImplementedInCore) {
        /* The vector permutations are implemented in librscore, so don't add them
         * to the white list.
         */
        for (ParameterDefinition *p : params) {
            if (p->mVectorSize != "1") {
                cerr << "#### " << p->rsType << ": " << p->mVectorSize << "\n";
                return;
            }
        }
    }
    string prototype;
    const string& functionName = permutation.getName();
    if (overloadable) {
        ostringstream stream;
        cerr << "****** " << functionName << "\n";
        stream << "_Z" << functionName.size() << functionName;
        writeParameters(&stream, params);
        prototype = stream.str();
    } else {
        prototype = function.getName();
    }
    entries->insert(prototype);
}

bool generateStubsWhiteList() {
    GeneratedFile file;
    if (!file.start(".",  "RSStubsWhiteList.cpp")) {
        return false;
    }

    set<string> entries;
    for (auto f : systemSpecification.getFunctions()) {
        const Function* function = f.second;
        for (auto spec : function->getSpecifications()) {
            bool overloadable = spec->isOverloadable();
            const string cpuImplementation = spec->getCpuImplementation();
            if (cpuImplementation != "core" && !spec->hasInline()) {
                bool vectorImplementedInCore = cpuImplementation == "vectorcore";
                for (auto permutation : spec->getPermutations()) {
                    writePrototype(&entries, *function, *permutation, overloadable,
                                   vectorImplementedInCore);
                }
            }
        }
    }

    // Write the comments that start the file.
    file.writeNotices();
    file << "#include \"RSStubsWhiteList.h\"\n\n";

    file << "std::vector<std::string> stubList = {\n";
    for (auto e : entries) {
        file << "\"" << e << "\",\n";
    }
    file << "};\n";

    return true;
}
