use crate::{
    ast::statement::LetStatement,
    codegen::ir::{IRState, IRWalkable},
    syntax_error::SyntaxError,
};

impl IRWalkable for LetStatement {
    type Output = ();

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        let register = match &self.expression {
            None => {
                ir.current_register += 1;
                ir.current_register
            }
            Some(expression) => expression.walk_ir(ir)?,
        };

        ir.scope
            .as_mut()
            .unwrap()
            .symbols
            .insert(&self.identifier, register);

        Ok(())
    }
}
