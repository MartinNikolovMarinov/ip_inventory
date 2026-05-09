set(__logger_prefix "" CACHE INTERNAL "Logging prefix")

function(init_logger log_prefix)
    set(__logger_prefix "${log_prefix}" CACHE INTERNAL "Logging prefix")
endfunction(init_logger)

function(log_info message)
    message(STATUS "${__logger_prefix} [INFO] ${message}")
endfunction(log_info)

function(log_warning message)
    message(WARNING "${__logger_prefix} [WARNING] ${message}")
endfunction(log_warning)

function(log_error message)
    message(SEND_ERROR "${__logger_prefix} [ERROR] ${message}")
endfunction(log_error)

function(log_fatal message)
    message(FATAL_ERROR "${__logger_prefix} [FATAL] ${message}")
endfunction(log_fatal)
