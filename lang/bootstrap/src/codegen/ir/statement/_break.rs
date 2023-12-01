use crate::{
    ast::statement::BreakStatement,
    codegen::ir::{IRState, IRStatement, IRWalkable, Label},
    syntax_error::SyntaxError,
};

impl IRWalkable for BreakStatement {
    type Output = ();

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        match ir.current_loop_break_label {
            Some(current_loop_break_label) => {
                ir.statements.push(IRStatement::Branch {
                    label: Label(current_loop_break_label),
                });

                Ok(())
            }
            None => Err(SyntaxError::BreakStatementOutsideLoop),
        }
    }
}
