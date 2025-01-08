//
// Created by Stuart Carnie on 6/8/2024.
//

#ifndef SLANG_ERROR_H
#define SLANG_ERROR_H

#include <filesystem>
#include <optional>
#include <string>
#include <variant>

namespace slang {
namespace error {
enum class Type {
	PARAMETER_MISMATCH,
	PATH_NOT_FOUND,
	GLSL_COMPILE,
	FAILED_MSG,
	PARSE,
	FAILED,
};

class Error;

using ParameterMismatch = std::u32string;
using PathNotFound = std::filesystem::path;
using UTF8String = std::string;

/**
 * An optional error type.
 */
using ErrorOpt = std::optional<Error>;

class Error {
	union {
		ParameterMismatch _parameter_mismatch;
		PathNotFound _path_not_found;
		UTF8String _utf8_string;
	};

public:
	Type type;

	static Error parameter_mismatch(const std::u32string &p_message) {
		Error err;
		err.type = Type::PARAMETER_MISMATCH;
		new (&err._parameter_mismatch) std::u32string(p_message);
		return err;
	}

	static Error failed() {
		Error err;
		err.type = Type::FAILED;
		return err;
	}

	static Error failed(std::string p_message) {
		Error err;
		err.type = Type::FAILED_MSG;
		new (&err._utf8_string) std::string(std::move(p_message));
		return err;
	}

	static Error failed(char const *p_message) {
		Error err;
		err.type = Type::FAILED_MSG;
		new (&err._utf8_string) std::string(p_message);
		return err;
	}

	static Error parse() {
		Error err;
		err.type = Type::PARSE;
		return err;
	}

	static Error glsl_compile(const std::string &p_message) {
		Error err;
		err.type = Type::GLSL_COMPILE;
		new (&err._utf8_string) std::string(p_message);
		return err;
	}

	static Error path_not_found(const std::filesystem::path &p_path) {
		Error err;
		err.type = Type::PATH_NOT_FOUND;
		new (&err._path_not_found) std::filesystem::path(p_path);
		return err;
	}

	std::string to_string() const;

	Error() {
		type = Type::FAILED;
	}

	Error(const Error &p_err) {
		type = p_err.type;
		switch (type) {
			case Type::PARAMETER_MISMATCH:
				new (&_parameter_mismatch) std::u32string(p_err._parameter_mismatch);
				break;
			case Type::PATH_NOT_FOUND:
				new (&_path_not_found) std::filesystem::path(p_err._path_not_found);
				break;
			case Type::GLSL_COMPILE:
				new (&_utf8_string) std::string(p_err._utf8_string);
				break;
			case Type::FAILED_MSG:
				new (&_utf8_string) std::string(p_err._utf8_string);
				break;
			case Type::PARSE:
				[[fallthrough]];
			case Type::FAILED:
				break;
		}
	}

	Error(Error &&p_error) {
		type = p_error.type;
		switch (type) {
			case Type::PARAMETER_MISMATCH:
				new (&_parameter_mismatch) std::u32string(std::move(p_error._parameter_mismatch));
				break;
			case Type::PATH_NOT_FOUND:
				new (&_path_not_found) std::filesystem::path(std::move(p_error._path_not_found));
				break;
			case Type::GLSL_COMPILE:
				new (&_utf8_string) std::string(std::move(p_error._utf8_string));
				break;
			case Type::FAILED_MSG:
				new (&_utf8_string) std::string(std::move(p_error._utf8_string));
				break;
			case Type::PARSE:
				[[fallthrough]];
			case Type::FAILED:
				break;
		}

		p_error.type = Type::FAILED;
	}

	~Error() {
		switch (type) {
			case Type::PARAMETER_MISMATCH:
				_parameter_mismatch.~basic_string();
				break;
			case Type::PATH_NOT_FOUND:
				_path_not_found.~path();
				break;
			case Type::GLSL_COMPILE:
				_utf8_string.~basic_string();
				break;
			case Type::FAILED_MSG:
				_utf8_string.~basic_string();
				break;
			case Type::PARSE:
				[[fallthrough]];
			case Type::FAILED:
				break;
		}
	}
};

} //namespace error

namespace result {

template <typename T>
class Result : public std::variant<T, error::Error> {
public:
	Result(const error::Error &p_error) :
			std::variant<T, error::Error>(p_error) {}
	Result(const T &p_value) :
			std::variant<T, error::Error>(p_value) {}

	static Result ok(const T &p_value) {
		return Result(p_value);
	}

	static Result err(const error::Error &p_error) {
		return Result(p_error);
	}

	bool is_ok() const {
		return std::holds_alternative<T>(*this);
	}

	bool is_err() const {
		return std::holds_alternative<error::Error>(*this);
	}

	/**
	 * Takes the value out of the result, leaving a default value in its place.
	 * @return T
	 */
	T take() {
		return std::move(std::get<T>(*this));
	}

	T &ok() {
		return std::get<T>(*this);
	}

	error::Error take_err() {
		return std::move(std::get<error::Error>(*this));
	}
};

template <typename T>
Result<T> ok(const T &p_value) {
	return Result<T>::ok(p_value);
}

template <typename T>
Result<T> err(const error::Error &p_error) {
	return Result<T>::err(p_error);
}

} //namespace result

} //namespace slang

#endif //SLANG_ERROR_H
