mod _assignment;
mod _break;
mod _continue;
mod _if;
mod _let;
mod _loop;
mod _while;

use super::{IRState, IRWalkable};
use crate::{ast::statement::Statement, syntax_error::SyntaxError};

impl IRWalkable for Statement {
    type Output = ();

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        match self {
            Statement::LetStatement(stmt) => stmt.walk_ir(ir),
            Statement::Assignment(stmt) => stmt.walk_ir(ir),
            Statement::IfStatement(stmt) => stmt.walk_ir(ir),
            Statement::BreakStatement(stmt) => stmt.walk_ir(ir),
            Statement::ContinueStatement(stmt) => stmt.walk_ir(ir),
            Statement::LoopStatement(stmt) => stmt.walk_ir(ir),
            Statement::WhileStatement(stmt) => stmt.walk_ir(ir),
            Statement::Expression(stmt) => match stmt.walk_ir(ir) {
                Ok(_) => Ok(()),
                Err(err) => Err(err),
            },
            Statement::EmptyStatement => Ok(()),
        }
    }
}
