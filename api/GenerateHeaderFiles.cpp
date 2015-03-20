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

// Convert a file name into a string that can be used to guard the include file with #ifdef...
static string makeGuardString(const string& filename) {
    string s;
    s.resize(15 + filename.size());
    s = "RENDERSCRIPT_";
    for (char c : filename) {
        if (c == '.') {
            s += '_';
        } else {
            s += toupper(c);
        }
    }
    return s;
}

// Write #ifdef's that ensure that the specified version is present
static void writeVersionGuardStart(GeneratedFile* file, VersionInfo info) {
    if (info.intSize == 32) {
        *file << "#ifndef __LP64__\n";
    } else if (info.intSize == 64) {
        *file << "#ifdef __LP64__\n";
    }

    if (info.minVersion <= 1) {
        // No minimum
        if (info.maxVersion > 0) {
            *file << "#if !defined(RS_VERSION) || (RS_VERSION <= " << info.maxVersion << ")\n";
        }
    } else {
        if (info.maxVersion == 0) {
            // No maximum
            *file << "#if (defined(RS_VERSION) && (RS_VERSION >= " << info.minVersion << "))\n";
        } else {
            *file << "#if (defined(RS_VERSION) && (RS_VERSION >= " << info.minVersion
                  << ") && (RS_VERSION <= " << info.maxVersion << "))\n";
        }
    }
}

static void writeVersionGuardEnd(GeneratedFile* file, VersionInfo info) {
    if (info.minVersion > 1 || info.maxVersion != 0) {
        *file << "#endif\n";
    }
    if (info.intSize != 0) {
        *file << "#endif\n";
    }
}

static void writeComment(GeneratedFile* file, const string& name, const string& briefComment,
                         const vector<string>& comment, bool closeBlock) {
    if (briefComment.empty() && comment.size() == 0) {
        return;
    }
    *file << "/*\n";
    if (!briefComment.empty()) {
        *file << " * " << name << ": " << briefComment << "\n";
        *file << " *\n";
    }
    for (size_t ct = 0; ct < comment.size(); ct++) {
        string s = stripHtml(comment[ct]);
        s = stringReplace(s, "@", "");
        if (!s.empty()) {
            *file << " * " << s << "\n";
        } else {
            *file << " *\n";
        }
    }
    if (closeBlock) {
        *file << " */\n";
    }
}

static void writeConstant(GeneratedFile* file, const Constant& constant) {
    const string name = constant.getName();
    writeComment(file, name, constant.getSummary(), constant.getDescription(), true);

    for (auto spec : constant.getSpecifications()) {
        VersionInfo info = spec->getVersionInfo();
        writeVersionGuardStart(file, info);
        *file << "#define " << name << " " << spec->getValue() << "\n";
        writeVersionGuardEnd(file, info);
    }
    *file << "\n";
}

static void writeTypeSpecification(GeneratedFile* file, const string& typeName,
                                   const TypeSpecification& spec) {
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

static void writeType(GeneratedFile* file, const Type& type) {
    const string name = type.getName();
    writeComment(file, name, type.getSummary(), type.getDescription(), true);

    for (auto spec : type.getSpecifications()) {
        writeTypeSpecification(file, name, *spec);
    }
    *file << "\n";
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

static void writeFunction(GeneratedFile* file, const Function& function) {
    // Write the generic documentation.
    writeComment(file, function.getName(), function.getSummary(), function.getDescription(), false);

    // Comment the parameters.
    if (function.someParametersAreDocumented()) {
        *file << " *\n";
        *file << " * Parameters:\n";
        for (auto p : function.getParameters()) {
            if (!p->documentation.empty()) {
                *file << " *   " << p->name << " " << p->documentation << "\n";
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

    // Write all the variants.
    for (auto spec : function.getSpecifications()) {
        for (auto permutation : spec->getPermutations()) {
            writeFunctionPermutation(file, *spec, *permutation);
        }
    }
}

static bool writeHeaderFile(const SpecFile& specFile) {
    const string headerFileName = specFile.getHeaderFileName();

    // We generate one header file for each spec file.
    GeneratedFile file;
    if (!file.start(headerFileName)) {
        return false;
    }

    // Write the comments that start the file.
    file.writeNotices();
    writeComment(&file, headerFileName, specFile.getBriefDescription(),
                 specFile.getFullDescription(), true);

    // Write the ifndef that prevents the file from being included twice.
    const string guard = makeGuardString(headerFileName);
    file << "#ifndef " << guard << "\n";
    file << "#define " << guard << "\n\n";

    // Add lines that need to be put in "as is".
    if (specFile.getVerbatimInclude().size() > 0) {
        for (auto s : specFile.getVerbatimInclude()) {
            file << s << "\n";
        }
        file << "\n";
    }

    /* Write the constants, types, and functions in the same order as
     * encountered in the spec file.
     */
    for (auto iter : specFile.getConstantsList()) {
        writeConstant(&file, *iter);
    }
    for (auto iter : specFile.getTypesList()) {
        writeType(&file, *iter);
    }
    for (auto iter : specFile.getFunctionsList()) {
        writeFunction(&file, *iter);
    }

    file << "#endif // " << guard << "\n";
    file.close();
    return true;
}

bool GenerateHeaderFiles() {
    bool success = true;
    for (auto specFile : systemSpecification.getSpecFiles()) {
        if (!writeHeaderFile(*specFile)) {
            success = false;
        }
    }
    return success;
}
