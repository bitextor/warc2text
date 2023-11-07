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

BOOST_AUTO_TEST_CASE(PreTagNotSupported) {
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

BOOST_AUTO_TEST_CASE(VoidTags) {
	std::string html("Text inside <embed>ignore <span>and me</span> me!</embed> is ignored");
	std::string expected("Test inside is ignored\n");
	AnnotatedText out;
	auto retval = processHTML(html, out, {});
	BOOST_CHECK_EQUAL(retval, util::SUCCESS);
	BOOST_CHECK_EQUAL(out.text, expected);
}

}
}

