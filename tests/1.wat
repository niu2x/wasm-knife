(module
	(memory $memory 1 16)
	(export "memory" (memory $memory))

	(func $hello (result i32)
		(i32.const 61))
	(export "hello" (func $hello))


)