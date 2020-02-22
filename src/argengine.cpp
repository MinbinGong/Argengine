// MIT License
//
// Copyright (c) 2020 Jussi Lind <jussi.lind@iki.fi>
//
// https://github.com/juzzlin/Argengine
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "argengine.hpp"

#include <cstdlib>
#include <iostream>
#include <map>

namespace juzzlin {

class Argengine::Impl
{
public:
    Impl(const ArgumentVector & args, bool addDefaultHelp)
      : m_args(args)
    {
        if (m_args.empty()) {
            throw std::runtime_error("Argument vector is empty!");
        }

        if (addDefaultHelp) {
            addHelp();
        }
    }

    void addArgument(ArgumentVariants argumentVariants, ValuelessCallback callback, bool required, std::string infoText)
    {
        if (const auto existing = getArgumentDefinition(argumentVariants)) {
            throw std::runtime_error("Argument '" + existing->getVariantsString() + "' already defined!");
        } else {
            const auto argumentDefinition = std::make_shared<ArgumentDefinition>(argumentVariants, callback, required, infoText);
            m_argumentDefinitions.push_back(argumentDefinition);
        }
    }

    void addHelp()
    {
        m_helpText = "Usage: " + m_args.at(0) + " [OPTIONS]";

        addArgument(
          { "-h", "--help" }, [=] {
              printHelp();
              exit(EXIT_SUCCESS);
          },
          false, "Show this help.");
    }

    ArgumentVector arguments() const
    {
        return m_args;
    }

    void setOutputStream(std::ostream & out)
    {
        m_out = &out;
    }

    void setErrorStream(std::ostream & err)
    {
        m_err = &err;
    }

    void setHelpText(std::string helpText)
    {
        m_helpText = helpText;
    }

    void setPositionalArgumentCallback(MultiStringCallback callback)
    {
        m_positionalArgumentCallback = callback;
    }

    void printHelp()
    {
        if (!m_helpText.empty()) {
            *m_out << m_helpText << std::endl
                   << std::endl;
        }

        *m_out << "Options:" << std::endl
               << std::endl;
    }

    void parse()
    {
        // TODO: Check required, Positional arguments
        for (size_t i = 1; i < m_args.size(); i++) {
            const auto arg = m_args.at(i);
            if (const auto match = getArgumentDefinition(arg)) {
                if (match->valuelessCallback) {
                    match->valuelessCallback();
                }
            } else {
                const auto warning = "Uknown argument '" + arg + "'!";
                switch (m_unknownArgumentBehavior) {
                case UnknownArgumentBehavior::Ignore:
                    break;
                case UnknownArgumentBehavior::Throw:
                    throw std::runtime_error(warning);
                case UnknownArgumentBehavior::Warn:
                    *m_err << warning << std::endl;
                    break;
                }
            }
        }
    }

    void setUnknownArgumentBehavior(UnknownArgumentBehavior behavior)
    {
        m_unknownArgumentBehavior = behavior;
    }

    ~Impl() = default;

private:
    struct ArgumentDefinition
    {
        ArgumentDefinition(const ArgumentVariants & variants, ValuelessCallback callback, bool required, std::string infoText)
          : variants(variants)
          , valuelessCallback(callback)
          , required(required)
          , infoText(infoText)
        {
        }

        std::string getVariantsString() const
        {
            std::string str;
            size_t count = 0;
            for (auto rit = variants.rbegin(); rit != variants.rend(); rit++) {
                str += *rit;
                if (++count < variants.size()) {
                    str += ", ";
                }
            }
            return str;
        }

        ArgumentVariants variants;

        ValuelessCallback valuelessCallback = nullptr;

        bool required = false;

        std::string infoText;
    };

    using ArgumentDefinitionPtr = std::shared_ptr<ArgumentDefinition>;

    ArgumentDefinitionPtr getArgumentDefinition(ArgumentVariants variants) const
    {
        for (auto && definition : m_argumentDefinitions) {
            for (auto && variant : variants) {
                if (definition->variants.count(variant)) {
                    return definition;
                }
            }
        }
        return nullptr;
    }

    ArgumentDefinitionPtr getArgumentDefinition(std::string argument) const
    {
        return getArgumentDefinition(ArgumentVariants { argument });
    }

    ArgumentVector m_args;

    std::string m_helpText;

    std::vector<ArgumentDefinitionPtr> m_argumentDefinitions;

    UnknownArgumentBehavior m_unknownArgumentBehavior = UnknownArgumentBehavior::Ignore;

    MultiStringCallback m_positionalArgumentCallback = nullptr;

    std::ostream * m_out = &std::cout;

    std::ostream * m_err = &std::cerr;
};

Argengine::Argengine(int argc, char ** argv, bool addDefaultHelp)
  : m_impl(new Impl(std::vector<std::string>(argv, argv + argc), addDefaultHelp))
{
}

Argengine::Argengine(const ArgumentVector & args, bool addDefaultHelp)
  : m_impl(new Impl(args, addDefaultHelp))
{
}

void Argengine::addArgument(ArgumentVariants argumentVariants, ValuelessCallback callback, bool required, std::string infoText)
{
    m_impl->addArgument(argumentVariants, callback, required, infoText);
}

Argengine::ArgumentVector Argengine::arguments() const
{
    return m_impl->arguments();
}

void Argengine::setHelpText(std::string helpText)
{
    m_impl->setHelpText(helpText);
}

void Argengine::setPositionalArgumentCallback(MultiStringCallback callback)
{
    m_impl->setPositionalArgumentCallback(callback);
}

void Argengine::setUnknownArgumentBehavior(UnknownArgumentBehavior behavior)
{
    m_impl->setUnknownArgumentBehavior(behavior);
}

void Argengine::setOutputStream(std::ostream & out)
{
    m_impl->setOutputStream(out);
}

void Argengine::setErrorStream(std::ostream & err)
{
    m_impl->setErrorStream(err);
}

void Argengine::printHelp()
{
    m_impl->printHelp();
}

void Argengine::parse()
{
    m_impl->parse();
}

Argengine::~Argengine() = default;

} // juzzlin
