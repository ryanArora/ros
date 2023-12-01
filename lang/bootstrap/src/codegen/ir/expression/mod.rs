mod binary_operation;
mod identifier;
mod literal;
mod unary_operation;

use super::{IRState, IRWalkable};
use crate::{ast::expression::Expression, syntax_error::SyntaxError};

impl IRWalkable for Expression {
    type Output = u32;

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        match self {
            Expression::BinaryOperation(binary_op) => binary_op.walk_ir(ir),
            Expression::UnaryOperation(unary_op) => unary_op.walk_ir(ir),
            Expression::Literal(literal) => literal.walk_ir(ir),
            Expression::Identifier(identifier) => identifier::ir_walk(ir, &identifier),
        }
    }
}
