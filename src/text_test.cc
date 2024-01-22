#include "text.hh"

#define BOOST_TEST_MODULE HTMLTest
#include <boost/test/unit_test.hpp>

namespace warc2text {
namespace {


BOOST_AUTO_TEST_CASE(TextPushBack) {
	AnnotatedText input;

	input.push_back("This is a sentence", "p");

	input.push_back("This is a sentence\nsplit over two lines", "li");

	std::string expected(
		"This is a sentence\n"
		"This is a sentence\n"
		"split over two lines\n"
	);

	std::vector<std::string> tags{"p", "li", "li"};

	BOOST_CHECK_EQUAL(input.text, expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(input.tags.begin(), input.tags.end(), tags.begin(), tags.end());
}

BOOST_AUTO_TEST_CASE(TextSubstr) {
	AnnotatedText input;
	input.push_back("This is a sentence", "p");
	input.push_back("This is a sentence\nsplit over two lines", "li");

	std::string expected(
		"This is a sentence\n"
		"This is a sent"
	);

	std::vector<std::string> tags{"p", "li"}; // two lines, so two tags

	AnnotatedText out = input.substr(0, expected.size());
	
	BOOST_CHECK_EQUAL(out.text, expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(out.tags.begin(), out.tags.end(), tags.begin(), tags.end());
}

BOOST_AUTO_TEST_CASE(TextAppend) {
	AnnotatedText input;
	input.push_back("This is a sentence", "p");
	input.push_back("This is a sentence\nsplit over two lines", "li");

	std::string expected(
		"This is a sentence over two lines\n"
	);

	std::vector<std::string> tags{"p"}; // single line, take existing tag

	AnnotatedText out;
	out.append(input, 0, 18); // "This is a sentence"
	out.append(input, 44, 16); // " over two lines\n"
	
	BOOST_CHECK_EQUAL(out.text, expected);
	BOOST_CHECK_EQUAL_COLLECTIONS(out.tags.begin(), out.tags.end(), tags.begin(), tags.end());
}

}
}

