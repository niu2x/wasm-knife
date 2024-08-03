(module
	(import "env" "memory" (memory 128))
	(data (i32.const 0) "hello world")

	(func $hello (result i32)
		(i32.const 61))
	(export "hello" (func $hello))
)