//  Copyright Joel de Guzman 2002-2004. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//  Hello World Example from the tutorial
//  [Joel de Guzman 10/9/2002]

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python.hpp>

char const* greet()
{
   return "hello, world";
}

struct World
{
    World(std::string m) : msg(m) {}
    void set(std::string msg) { this->msg = msg; }
    std::string greet() { return msg; }
    std::string msg;
};

BOOST_PYTHON_MODULE(boostpy)
{
    boost::python::def("greet", greet);
    boost::python::class_<World>("World").def("greet", &World::greet).def("set", &World::set);
}