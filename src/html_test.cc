#include "html.hh"

#define BOOST_TEST_MODULE HTMLTest
#include <boost/test/unit_test.hpp>

namespace warc2text {
namespace {


BOOST_AUTO_TEST_CASE(CleanHTML) {
	std::string html(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"	<head>\n"
		"		<title>Well-formed web page!</title>\n"
		"	</head>\n"
		"	<body>\n"
		"		<p>This is a paragraph.</p>\n"
		"		<p>\n"
		"			This is &lt;one&gt;,\n"
		"			indented as written by <a href=\"\">Ken</a>,\n"
		"			with a newline.\n"
		"		</p>\n"
		"	</body>\n"
		"</html>");

	std::string expected(
		"Well-formed web page!\n"
		"This is a paragraph.\n"
		"This is <one>, indented as written by Ken, with a newline.\n"
	);

	std::vector<std::string> tags{"title", "p", "p"};

	AnnotatedText out;
	auto retval = processHTML(html, out, {});

	BOOST_CHECK_EQUAL(retval, util::SUCCESS);
	BOOST_CHECK_EQUAL(out.text, expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(out.tags.begin(), out.tags.end(), tags.begin(), tags.end());
}

BOOST_AUTO_TEST_CASE(TagsIdentifiers) {
	// breaks because we don't have a stack, so after </p> but before <p> no idea
	// we're inside <div>.
	std::string html(
		"<div>\n"
		"  <p>Text</p>\n"
		"  <span>not block text</span>\n"
		"  <embed>alt text</embed>\n"
		"  <p>Paragraph</p>\n"
		"</div>"
	);

	std::string expected(
		"Text\n"
		"not block text alt text\n"
		"Paragraph\n"
	);

	std::vector<std::string> tags{"p", "div", "p"};

	AnnotatedText out;
	auto retval = processHTML(html, out, {});

	BOOST_CHECK_EQUAL(retval, util::SUCCESS);
	BOOST_CHECK_EQUAL(out.text, expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(out.tags.begin(), out.tags.end(), tags.begin(), tags.end());
}

BOOST_AUTO_TEST_CASE(PreTagNotSupported) {
	// We don't support keeping the formatting in <pre> tags.
	std::string html(
		"<pre> This line\n"
		"should keep its newlines\n"
		"ideally.</pre>");

	std::string expected("This line should keep its newlines ideally.\n");

	std::vector<std::string> tags{"pre"};

	AnnotatedText out;
	auto retval = processHTML(html, out, {});

	BOOST_CHECK_EQUAL(retval, util::SUCCESS);
	BOOST_CHECK_EQUAL(out.text, expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(out.tags.begin(), out.tags.end(), tags.begin(), tags.end());
}

BOOST_AUTO_TEST_CASE(BlockTags) {
	std::string html("<body>Alpha <br> Beta <h1> Gamma </h1> Delta <span> Epsilon </span> Zeta</body>");
	std::string expected("Alpha\nBeta\nGamma\nDelta Epsilon Zeta\n");
	AnnotatedText out;
	auto retval = processHTML(html, out, {});
	BOOST_CHECK_EQUAL(retval, util::SUCCESS);
	BOOST_CHECK_EQUAL(out.text, expected);
}

BOOST_AUTO_TEST_CASE(VoidTags) {
	std::string html("<body>Void<img>tags<img>should<img>add<embed>beep</embed>spaces</body>");
	std::string expected("Void tags should add beep spaces\n");
	AnnotatedText out;
	auto retval = processHTML(html, out, {});
	BOOST_CHECK_EQUAL(retval, util::SUCCESS);
	BOOST_CHECK_EQUAL(out.text, expected);
}

BOOST_AUTO_TEST_CASE(ScriptTags, *boost::unit_test::disabled()) {
	// This fails since we don't keep a stack, so ` is ignored` will still
	// have tag_name == "script" and be ignored :facepalm:
	std::string html("<body>Text inside <script>ignore <span>and me</span> me!</script> is ignored</body>");
	std::string expected("Test inside is ignored\n");
	AnnotatedText out;
	auto retval = processHTML(html, out, {});
	BOOST_CHECK_EQUAL(retval, util::SUCCESS);
	BOOST_CHECK_EQUAL(out.text, expected);
}

}
}

