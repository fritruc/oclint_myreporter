#include <ctime>
#include <map>
#include <iostream>

#include "oclint/Results.h"
#include "oclint/Reporter.h"
#include "oclint/RuleBase.h"
#include "oclint/Version.h"
#include "oclint/ViolationSet.h"

using namespace oclint;

class HTMLReporter : public Reporter
{
public:
    virtual const std::string name() const override
    {
        return "html";
    }

    virtual void report(Results* results, std::ostream& out) override
    {
        out << "<!DOCTYPE html>";
        out << "<html>";
        writeHead(out);
        out << "<body>";
        out << "<h1>OCLint Report</h1>";
        out << "<hr />";
        out << "<h2>Summary</h2>";
        writeSummaryTable(out, *results);
        out << "<hr />";
        out << "<table class=\"sortable\"><thead><tr><th>File</th><th>Location</th>"
            << "<th>Rule Name</th><th>Rule Category</th>"
            << "<th>Priority</th><th>Message</th></tr></thead><tbody>";
        for (const auto& violation : results->allViolations())
        {
            writeViolation(out, violation);
        }
        if (results->hasErrors())
        {
            writeCompilerDiagnostics(out, results->allErrors(), "error");
        }
        if (results->hasWarnings())
        {
            writeCompilerDiagnostics(out, results->allWarnings(), "warning");
        }
        if (results->hasCheckerBugs())
        {
            writeCheckerBugs(out, results->allCheckerBugs());
        }
        out << "</tbody></table>";
        out << "<hr />";
        writeFooter(out, Version::identifier());
        out << "</body>";
        out << "</html>";
        out << std::endl;
    }

    void writeFooter(std::ostream &out, std::string version)
    {
        time_t now = time(nullptr);
        out << "<p>" << ctime(&now)
            << "| Generated with <a href='http://oclint.org'>OCLint v" << version << "</a>.</p>";
    }

    void writeViolation(std::ostream &out, const Violation &violation)
    {
        const RuleBase *rule = violation.rule;

        std::string  classname = rule->name();
        std::replace(classname.begin(), classname.end(), ' ', '_' );
        out << "<tr class=\"" << classname << "\">"
            << "<td>" << violation.path << "</td><td>" << violation.startLine
            << ":" << violation.startColumn << "</td>";
        out << "<td>" << rule->name() << "</td>"
            << "<td>" << rule->category() << "</td>"
            << "<td class='priority" << rule->priority() << "'>"
            << rule->priority() << "</td>"
            << "<td>" << violation.message << "</td></tr>";
    }

    void writeCompilerErrorOrWarning(std::ostream &out,
        const Violation &violation, std::string level)
    {
        out << "<tr class=\"compiler-" << level << "\">"
            << "<td>" << violation.path << "</td><td>" << violation.startLine
            << ":" << violation.startColumn << "</td>";
        out << "<td>compiler " << level << "</td><td></td><td class='cmplr-" << level << "'>"
            << level << "</td><td>" << violation.message << "</td></tr>";
    }

    void writeCompilerDiagnostics(std::ostream &out, std::vector<Violation> violations,
        std::string level)
    {
        for (const auto& violation : violations)
        {
            writeCompilerErrorOrWarning(out, violation, level);
        }
    }

    void writeCheckerBugs(std::ostream &out, std::vector<Violation> violations)
    {
        for (const auto& violation : violations)
        {
            out << "<tr class=\"clang_static_analyzer\"><td>" << violation.path << "</td><td>" << violation.startLine
                << ":" << violation.startColumn << "</td>";
            out << "<td>clang static analyzer</td><td></td><td class='checker-bug'>"
                << "checker bug</td><td>" << violation.message << "</td></tr>";
        }
    }

    void writeSummaryTable(std::ostream &out, Results &results)
    {
        // out << "<table><thead><tr><th>Total Files</th><th>Files with Violations</th>"
        //     << "<th>Priority 1</th><th>Priority 2</th><th>Priority 3</th>"
        //     << "<th>Compiler Errors</th><th>Compiler Warnings</th>"
        //     << "<th>Clang Static Analyzer</th></tr></thead>";
        // out << "<tbody><tr><td>" << results.numberOfFiles() << "</td><td>"
        //     << results.numberOfFilesWithViolations() << "</td><td class='priority1'>"
        //     << results.numberOfViolationsWithPriority(1) << "</td><td class='priority2'>"
        //     << results.numberOfViolationsWithPriority(2) << "</td><td class='priority3'>"
        //     << results.numberOfViolationsWithPriority(3) << "</td><td class='cmplr-error'>"
        //     << results.allErrors().size() << "</td><td class='cmplr-warning'>"
        //     << results.allWarnings().size() << "</td><td class='checker-bug'>"
        //     << results.allCheckerBugs().size() << "</td></tr></tbody></table>";

        auto  fct = [](std::ostream &out, Results&  results,  int  priority ) {
            std::map< std::string, int >  array;  
            for (const auto& violation : results.allViolations())
            {
                const RuleBase *rule = violation.rule;

                if( rule->priority() != priority )
                    continue;
        
                std::string  classname = rule->name();
                std::replace(classname.begin(), classname.end(), ' ', '_' );
                
                auto  element = array.find(classname);
                if( element == array.end())
                {
                    array[classname] = 1;
                    continue;
                }
                
                element->second += 1;
            }
            
            out << "<tr><th class='priority"<< priority << "'>Priority " << priority << "</th><th colspan=\"2\" class='priority"<< priority << "'>" << results.numberOfViolationsWithPriority(priority) << "</th></tr>";
            
            for( auto  it = array.begin(); it != array.end(); ++it )
            {
                out << "<tr><td class=\"SUMM_DESC\">" << it->first << "</td><td class=\"Q\">" << it->second << "</td><td><center><input type=\"checkbox\" onclick=\"ToggleDisplay(this,&#39;"<< it->first <<"&#39;);\" checked=\"\"></center></td></tr>";
            }

        };

        out << "<h2>Bug Summary</h2>";
        out << "<table><thead><tr><td>Bug Type</td><td>Quantity</td><td>Display?</td></tr></thead>";
        out << "<tbody><tr style=\"font-weight:bold\"><td class=\"SUMM_DESC\">All Bugs</td><td>"<< results.numberOfViolationsWithPriority(1)+results.numberOfViolationsWithPriority(2)+results.numberOfViolationsWithPriority(3) <<"</td><td><center><input type=\"checkbox\" id=\"AllBugsCheck\" onclick=\"CopyCheckedStateToCheckButtons(this);\" checked=\"\"></center></td></tr>";
        if( results.numberOfViolationsWithPriority(1) > 0 )
        {
            fct( out, results, 1 );
        }
        if( results.numberOfViolationsWithPriority(2) > 0 )
        {
            fct( out, results, 2 );
        }
        if( results.numberOfViolationsWithPriority(3) > 0 )
        {
            fct( out, results, 3 );
        }

        out << "</tbody></table>";
    }

    void writeHead(std::ostream &out)
    {
        out << "<head>";
        out << "<title>OCLint Report</title>";
        out << "<style type='text/css'>"
            << "                             \
.priority1, .priority2, .priority3,          \
.cmplr-error, .cmplr-warning, .checker-bug { \
    font-weight: bold;                       \
    text-align: center;                      \
}                                            \
.priority1, .priority2, .priority3 {         \
    color: #BF0A30;                          \
}                                            \
.priority1 { background-color: #FFC200; }    \
.priority2 { background-color: #FFD3A6; }    \
.priority3 { background-color: #FFEEB5; }    \
.cmplr-error, .cmplr-warning {               \
    background-color: #BF0A30;               \
}                                            \
.cmplr-error { color: #FFC200; }             \
.cmplr-warning { color: #FFD3A6; }           \
.checker-bug {                               \
    background-color: #002868;               \
    color: white;                            \
}                                            \
table {                                      \
    border: 2px solid gray;                  \
    border-collapse: collapse;               \
    -moz-box-shadow: 3px 3px 4px #AAA;       \
    -webkit-box-shadow: 3px 3px 4px #AAA;    \
    box-shadow: 3px 3px 4px #AAA;            \
}                                            \
td, th {                                     \
    border: 1px solid #D3D3D3;               \
    padding: 4px 20px 4px 20px;              \
}                                            \
th {                                         \
    text-shadow: 2px 2px 2px white;          \
    border-bottom: 1px solid gray;           \
    background-color: #E9F4FF;               \
}"
            << "</style>";
            out << "<script src=\"sorttable.js\"></script>";

            out << "<script language=\"javascript\" type=\"text/javascript\">\
function SetDisplay(RowClass, DisplayVal)\
{\
  var Rows = document.getElementsByTagName(\"tr\");\
  for ( var i = 0 ; i < Rows.length; ++i ) {\
    if (Rows[i].className == RowClass) {\
      Rows[i].style.display = DisplayVal;\
    }\
  }\
}\
\
function CopyCheckedStateToCheckButtons(SummaryCheckButton) {\
  var Inputs = document.getElementsByTagName(\"input\");\
  for ( var i = 0 ; i < Inputs.length; ++i ) {\
    if (Inputs[i].type == \"checkbox\") {\
      if(Inputs[i] != SummaryCheckButton) {\
        Inputs[i].checked = SummaryCheckButton.checked;\
        Inputs[i].onclick();\
      }\
    }\
  }\
}\
\
function returnObjById( id ) {\
    if (document.getElementById)\
        var returnVar = document.getElementById(id);\
    else if (document.all)\
        var returnVar = document.all[id];\
    else if (document.layers)\
        var returnVar = document.layers[id];\
    return returnVar;\
}\
\
var NumUnchecked = 0;\
\
function ToggleDisplay(CheckButton, ClassName) {\
  if (CheckButton.checked) {\
    SetDisplay(ClassName, \"\");\
    if (--NumUnchecked == 0) {\
      returnObjById(\"AllBugsCheck\").checked = true;\
    }\
  }\
  else {\
    SetDisplay(ClassName, \"none\");\
    NumUnchecked++;\
    returnObjById(\"AllBugsCheck\").checked = false;\
  }\
}\
</script>";
        out << "</head>";
    }
};

extern "C" Reporter* create()
{
  return new HTMLReporter();
}
