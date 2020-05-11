#include <optional>
#include <variant>
#include <stdexcept>
#include <iostream>

template<typename T, typename E>
class expected
{
private:
    union {
        T m_value;
        E m_error{};
    };
    bool m_valid;

public:
    expected() = default;

    T& value()
    {
        if(!m_valid){
            throw std::logic_error("Missing a value");
        }
        return m_value;
    }

    T value() const
    {
        if(!m_valid){
            throw std::logic_error("Missing a value");
        }
        return m_value;
    }

    E& error()
    {
        if(m_valid){
            throw std::logic_error("There is no an error");
        }
        return m_error;
    }

    E error() const
    {
        if(m_valid){
            throw std::logic_error("There is no error");
        }
        return m_error;
    }

    template<typename... Args>
    static expected success(Args&&... params)
    {
        //create uninitialized union
        expected result;
        // initialize union tag. You have valid value inside
        result.m_valid = true;
        // Call placement new to initialize the value of type T in memory
        new (&result.m_value) T(std::forward<Args>(params)...);
        return result;
    }

    template<typename... Args>
    static expected error(Args&&... params)
    {
        expected result;
        // initialize union tag. You have an error inside
        result.m_valid = false;
        // Call placement new to initialize the value of type E in memory
        new (&result.m_error) E(std::forward<Args>(params)...);
        return result;
    }

    ~expected()
    {
        // Explicit calling dtor of inside type of union
        if(m_valid){
            m_value.~T();
        }
        else {
            m_error.~E();
        }
    }

    expected(const expected& other) : m_valid(other.m_valid)
    {
        if(m_valid) {
            new (&m_value) T(other.m_value);
        }
        else {
            new (&m_error) E(other.m_error);
        }
    }

    expected(expected&& other) : m_valid(other.m_valid)
    {
        if(m_valid) {
            new (&m_value) T(std::move(other.m_value));
        }
        else {
            new (&m_error) E(std::move(other.m_error));
        }
    }

    void swap(expected& other)
    {
        using std::swap;
        if(m_valid){
            if(other.m_valid){
                swap(m_value, other.m_value);
            }
            else{
                auto temp = std::move(other.m_error);
                other.m_error.~E();
                new (&other.m_value) T(std::move(m_value));
                m_value.~T();
                new (&m_error) E(std::move(temp));
                std::swap(m_valid, other.m_valid);
            }
        }
        else {
            if(other.m_valid){
                other.swap(*this);
            }
            else{
                swap(m_error, other.m_error);
            }
        }
    }

    expected& operator=(expected other)
    {
        swap(other);
        return *this;
    }

    operator bool() const
    {
        return m_valid;
    }

    operator std::optional<T>() const
    {
        if(m_valid){
            return m_value;
        }
        else{
            return std::optional<T>();
        }
    }
};

template <typename T, 
		  typename Variant,    
		  typename Expected = expected<T, std::string>>
Expected  get_if(const Variant& variant)
{    
	const T* ptr = std::get_if<T>(&variant);
	if (ptr) {
		return Expected::success(*ptr);    
	} else {  
		return Expected::error("Variant doesn't contain the desired type");
	}
}

expected<int, std::string> foo(bool valid)
{
    if(valid){
        return expected<int, std::string>::success(10);
    }
    else{
        return expected<int, std::string>::error(std::string{"foo error"});
    }
}

auto bar(bool flag) ->std::variant<int, std::string> 
{
	if(flag){
		return std::variant<int, std::string> {55};
	}
	else{
		return std::variant<int, std::string> {"error"};
	}
}

int main()
{
	for(bool flag : {true, false}) {
		
		auto res1 = foo(flag);
		if(res1){
			std::cout << "value1 is " << res1.value() << std::endl;
		}
		else {
			std::cout << "error1 is \"" << res1.error() << "\"" << std::endl;
		}

		const auto& ret_bar = bar(flag);
		auto res2 = get_if<int>(ret_bar);
		if(res2){
			std::cout << "value2 is " << res2.value() << std::endl;
		}
		else {
			std::cout << "error2 is \"" << res2.error() << "\"" << std::endl;
		}
		
	}
}
