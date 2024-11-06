#ifndef CONFIG_VALUE_HPP
# define CONFIG_VALUE_HPP

template<typename T>
class ConfigValue {
private:
    T _value;
    bool _is_set;

public:
    // Constructors
    ConfigValue();
    explicit ConfigValue(const T& value);
    ConfigValue(const ConfigValue& other);

    // Assignment operator
    ConfigValue& operator=(const ConfigValue& other);

    // Destructor
    ~ConfigValue();

    // Methods
    void set(const T& value);
    bool isSet() const;
    const T& getValue() const;
    T& getValue();
    void clear();

    // Optional: direct value access operator
    operator T() const;
};

// Template implementation must be in header
# include "ConfigValue.tpp"

#endif
