use crate::{
    ast::statement::ContinueStatement,
    codegen::ir::{IRState, IRStatement, IRWalkable, Label},
    syntax_error::SyntaxError,
};

impl IRWalkable for ContinueStatement {
    type Output = ();

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        match ir.current_loop_continue_label {
            Some(current_loop_continue_label) => {
                ir.statements.push(IRStatement::Branch {
                    label: Label(current_loop_continue_label),
                });

                Ok(())
            }
            None => Err(SyntaxError::ContinueStatementOutsideLoop),
        }
    }
}
