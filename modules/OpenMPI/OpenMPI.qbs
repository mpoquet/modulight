import qbs 1.0

Module
{
	name: 'OpenMPI'

	Depends{name:'cpp'}

	cpp.includePaths: ['/usr/lib/openmpi/include', '/usr/lib/openmpi/include/openmpi']
	
	cpp.libraryPaths: ['/usr/lib/openmpi/lib']
	cpp.dynamicLibraries: ['mpi_cxx', 'mpi', 'open-rte', 'open-pal', 'dl', 'nsl', 'util', 'm', 'dl']
}
