use crate::{
    codegen::ir::{get_identifier_register, IRState},
    syntax_error::SyntaxError,
};

pub fn ir_walk<'a>(ir: &mut IRState<'a>, identifier: &'a str) -> Result<u32, SyntaxError> {
    match get_identifier_register(ir.scope.clone(), identifier) {
        None => Err(SyntaxError::UndefinedReference),
        Some(register) => Ok(register),
    }
}
