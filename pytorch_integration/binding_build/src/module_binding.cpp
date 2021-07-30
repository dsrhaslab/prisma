#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "client.h"
#include "server.h"
#include "prisma.h"

#include <errno.h>

namespace py = pybind11;

//Two functions that are equal. Needs an interface classe to use the same function for both purposes.

//return is not dealt properly
int read(Client& c, const std::string& filename, size_t n, char* buffer){
	size_t offset = 0;
	while(n > 0){
		size_t requested_read_length;
		if (n > INT32_MAX) 
			requested_read_length = INT32_MAX;
		else
			requested_read_length = n;
		
		ssize_t s = c.read(filename, buffer + offset, requested_read_length, offset);
		
		if(s > 0){
			n-=s;
			offset += s;
		}else if (s == 0) {
				return -1;
			} else if (errno == EINTR || errno == EAGAIN) {
				// Retry
			} else {
				return -1;
			}
	}
	return n;
}

int read(Prisma& prisma, const std::string& filename, size_t n, char* buffer){
	size_t offset = 0;
	while(n > 0){
		size_t requested_read_length;
		if (n > INT32_MAX) 
			requested_read_length = INT32_MAX;
		else
			requested_read_length = n;
		
		ssize_t s = prisma.read(filename, buffer + offset, requested_read_length, offset);
		
		if(s > 0){
			n-=s;
			offset += s;
		}else if (s == 0) {
				return -1;
			} else if (errno == EINTR || errno == EAGAIN) {
				// Retry
			} else {
				return -1;
			}
	}
	return n;
}


PYBIND11_MODULE(prisma_binding, m)
{
	m.doc() = "prisma_binding";

	py::class_<Client>(m, "PrismaClient")
		.def(py::init<>())
		.def("read", [](Client& c, const std::string& filename, size_t n) {
			PyBytesObject* bytesObject = (PyBytesObject*) PyObject_Malloc(offsetof(PyBytesObject, ob_sval) + n);
			PyObject_INIT_VAR(bytesObject, &PyBytes_Type, n);
			bytesObject->ob_shash = -1;
			int status = read(c, filename, n, bytesObject->ob_sval);
			return std::tuple(status, py::reinterpret_steal<py::object>((PyObject*)bytesObject));
		}, py::return_value_policy::take_ownership);

	py::class_<Server>(m, "PrismaServer")
		.def(py::init<>())
		.def("run", &Server::run);

	py::class_<Prisma>(m, "Prisma")
		.def(py::init<>())
		.def("read", [](Prisma& prisma, const std::string& filename, size_t n) {
			PyBytesObject* bytesObject = (PyBytesObject*) PyObject_Malloc(offsetof(PyBytesObject, ob_sval) + n);
			PyObject_INIT_VAR(bytesObject, &PyBytes_Type, n);
			bytesObject->ob_shash = -1;
			int status = read(prisma, filename, n, bytesObject->ob_sval);
			return std::tuple(status, py::reinterpret_steal<py::object>((PyObject*)bytesObject));
		}, py::return_value_policy::take_ownership);
}
