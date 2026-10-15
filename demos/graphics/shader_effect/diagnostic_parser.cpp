// SPDX-License-Identifier: MS-PL
// Turn renderer compiler text into owned, portable diagnostics and one stable exception summary.
#include "CNA/ShaderDiagnosticEXT.hpp"
#include "CnaDemos/Demo.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
    template<class Action>
    bool RefusesInvalidArgument(Action&& action)
    {
        try
        {
            std::forward<Action>(action)();
        }
        catch (const std::invalid_argument&)
        {
            return true;
        }
        return false;
    }

    void DiagnosticParser(CnaDemos::DemoContext& ctx)
    {
        std::string label = "authored.vert";
        std::string message = "undeclared identifier";
        const CNA::ShaderDiagnosticEXT authored(
            CNA::ShaderDiagnosticSeverityEXT::Error, CNA::ShaderStageEXT::Vertex,
            label, 7, 11, message);
        label.assign("overwritten");
        message.assign("overwritten");
        ctx.Check(authored.getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Error &&
                      authored.getStage() == CNA::ShaderStageEXT::Vertex &&
                      authored.getSourceLabel() == "authored.vert" && authored.getLine() == 7 &&
                      authored.getColumn() == 11 && authored.getMessage() == "undeclared identifier",
                  "a diagnostic validates and owns its severity, stage, label, location and message");

        std::string log =
            "VS: shader.vert:4:7: warning: implicit conversion\n"
            " info: compiler selected fast path \r\n"
            "FS: ERROR: 0:12: undeclared identifier\n"
            "FS: 0:31(2): error: missing output\n"
            "shader.frag(20,3): warning: precision loss\n"
            "plain failure";
        std::vector<CNA::ShaderDiagnosticEXT> parsed = CNA::ShaderDiagnosticEXT::parseCompilerLog(
            log, CNA::ShaderStageEXT::Unknown, "portable-package");
        log.assign("overwritten");

        const bool labelsAndMessagesOwned = parsed.size() == 6 &&
            parsed[0].getSourceLabel() == "portable-package" &&
            parsed[0].getMessage() == "VS: shader.vert:4:7: warning: implicit conversion" &&
            parsed[1].getMessage() == "info: compiler selected fast path" &&
            parsed[5].getMessage() == "plain failure";
        ctx.Check(labelsAndMessagesOwned,
                  "each nonblank compiler-log line becomes an independently owned trimmed record");

        const bool firstFormsMatch = parsed.size() == 6 &&
            parsed[0].getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Warning &&
            parsed[0].getStage() == CNA::ShaderStageEXT::Vertex &&
            parsed[0].getLine() == 4 && parsed[0].getColumn() == 7 &&
            parsed[1].getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Information &&
            parsed[1].getStage() == CNA::ShaderStageEXT::Vertex &&
            parsed[1].getLine() == 0 && parsed[1].getColumn() == 0 &&
            parsed[2].getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Error &&
            parsed[2].getStage() == CNA::ShaderStageEXT::Fragment &&
            parsed[2].getLine() == 12 && parsed[2].getColumn() == 0;
        ctx.Check(firstFormsMatch,
                  "colon locations, numeric source IDs and a continuation inherit exact severity and stage");

        const bool parenthesizedFormsMatch = parsed.size() == 6 &&
            parsed[3].getStage() == CNA::ShaderStageEXT::Fragment &&
            parsed[3].getLine() == 31 && parsed[3].getColumn() == 2 &&
            parsed[4].getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Warning &&
            parsed[4].getStage() == CNA::ShaderStageEXT::Fragment &&
            parsed[4].getLine() == 20 && parsed[4].getColumn() == 3 &&
            parsed[5].getSeverity() == CNA::ShaderDiagnosticSeverityEXT::Error &&
            parsed[5].getStage() == CNA::ShaderStageEXT::Fragment &&
            parsed[5].getLine() == 0 && parsed[5].getColumn() == 0;
        ctx.Check(parenthesizedFormsMatch,
                  "both parenthesized location forms parse and later unmarked lines retain the current stage");

        ctx.Check(CNA::ShaderDiagnosticEXT::parseCompilerLog(
                      " \t\r\n\n", CNA::ShaderStageEXT::Fragment, "empty").empty(),
                  "an empty or whitespace-only compiler log produces no invented diagnostic");

        const bool invalidValuesRefused =
            RefusesInvalidArgument([] {
                CNA::ShaderDiagnosticEXT value(CNA::ShaderDiagnosticSeverityEXT::Count,
                    CNA::ShaderStageEXT::Vertex, {}, 0, 0, "message");
            }) &&
            RefusesInvalidArgument([] {
                CNA::ShaderDiagnosticEXT value(CNA::ShaderDiagnosticSeverityEXT::Error,
                    CNA::ShaderStageEXT::Count, {}, 0, 0, "message");
            }) &&
            RefusesInvalidArgument([] {
                CNA::ShaderDiagnosticEXT value(CNA::ShaderDiagnosticSeverityEXT::Error,
                    CNA::ShaderStageEXT::Vertex, {}, -1, 0, "message");
            }) &&
            RefusesInvalidArgument([] {
                CNA::ShaderDiagnosticEXT value(CNA::ShaderDiagnosticSeverityEXT::Error,
                    CNA::ShaderStageEXT::Vertex, {}, 0, 1, "message");
            }) &&
            RefusesInvalidArgument([] {
                CNA::ShaderDiagnosticEXT value(CNA::ShaderDiagnosticSeverityEXT::Error,
                    CNA::ShaderStageEXT::Vertex, {}, 0, 0, {});
            });
        ctx.Check(invalidValuesRefused,
                  "invalid identities, locations and empty messages are refused by the value constructor");
        ctx.Check(RefusesInvalidArgument([] {
                      (void) CNA::ShaderDiagnosticEXT::parseCompilerLog(
                          "error", CNA::ShaderStageEXT::Count, "invalid");
                  }),
                  "the parser refuses an invalid fallback stage before consuming the log");

        CNA::ShaderCompilationExceptionEXT failure(parsed);
        parsed.clear();
        const std::string summary = failure.what();
        ctx.Check(failure.getDiagnostics().size() == 6 &&
                      failure.getDiagnostics()[4].getLine() == 20 &&
                      summary.find("6 diagnostics") != std::string::npos &&
                      summary.find("Warning Vertex 'portable-package':4:7") != std::string::npos &&
                      summary.ends_with("; 5 more"),
                  "the compilation exception owns every record and summarizes the first with the remaining count");
        ctx.Check(RefusesInvalidArgument([] {
                      CNA::ShaderCompilationExceptionEXT failure(
                          std::vector<CNA::ShaderDiagnosticEXT>{});
                  }),
                  "a compilation exception refuses an empty diagnostic list");

        ctx.Print("Parsed {} owned diagnostics; summary: {}",
                  failure.getDiagnostics().size(), failure.what());
    }
}

CNA_DEMOS_CONSOLE(DiagnosticParser, {
    .id = "graphics.shader_effect.diagnostic_parser",
    .title = "Portable shader diagnostic parser",
    .description = "Parses common compiler-log location and stage forms into owned diagnostics, validates authored "
                   "records, and preserves the complete list behind one stable exception summary.",
    .apis = {"CNA::ShaderDiagnosticEXT::ShaderDiagnosticEXT", "CNA::ShaderDiagnosticEXT::parseCompilerLog",
             "CNA::ShaderDiagnosticEXT::getSeverity", "CNA::ShaderDiagnosticEXT::getStage",
             "CNA::ShaderDiagnosticEXT::getSourceLabel", "CNA::ShaderDiagnosticEXT::getLine",
             "CNA::ShaderDiagnosticEXT::getColumn", "CNA::ShaderDiagnosticEXT::getMessage",
             "CNA::ShaderDiagnosticSeverityEXT", "CNA::ShaderStageEXT",
             "CNA::ShaderCompilationExceptionEXT::ShaderCompilationExceptionEXT",
             "CNA::ShaderCompilationExceptionEXT::getDiagnostics"},
    .tags = {"shader", "diagnostics", "compiler", "parser", "cnaext"},
})
