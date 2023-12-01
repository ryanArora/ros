use crate::{
    ast::expression::{BinaryOperation, BinaryOperationType},
    codegen::ir::{IRState, IRStatement, IRWalkable, Register},
    syntax_error::SyntaxError,
};

impl IRWalkable for BinaryOperation {
    type Output = u32;

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        let left = self.left_expression.walk_ir(ir)?;
        let right = self.right_expression.walk_ir(ir)?;

        match self.operation_type {
            BinaryOperationType::Add => {
                ir.current_register += 1;
                ir.statements.push(IRStatement::Add {
                    rd: Register(ir.current_register),
                    rs1: Register(left),
                    rs2: Register(right),
                });
                Ok(ir.current_register)
            }
            BinaryOperationType::Subtract => {
                ir.current_register += 1;
                ir.statements.push(IRStatement::Subtract {
                    rd: Register(ir.current_register),
                    rs1: Register(left),
                    rs2: Register(right),
                });
                Ok(ir.current_register)
            }
            BinaryOperationType::BitwiseAnd => {
                ir.current_register += 1;
                ir.statements.push(IRStatement::And {
                    rd: Register(ir.current_register),
                    rs1: Register(left),
                    rs2: Register(right),
                });
                Ok(ir.current_register)
            }
            BinaryOperationType::BitwiseOr => {
                ir.current_register += 1;
                ir.statements.push(IRStatement::Or {
                    rd: Register(ir.current_register),
                    rs1: Register(left),
                    rs2: Register(right),
                });
                Ok(ir.current_register)
            }
            BinaryOperationType::BitwiseXor => {
                ir.current_register += 1;
                ir.statements.push(IRStatement::Xor {
                    rd: Register(ir.current_register),
                    rs1: Register(left),
                    rs2: Register(right),
                });
                Ok(ir.current_register)
            }
            BinaryOperationType::LeftShift => {
                ir.current_register += 1;
                ir.statements.push(IRStatement::LeftShift {
                    rd: Register(ir.current_register),
                    rs1: Register(left),
                    rs2: Register(right),
                });
                Ok(ir.current_register)
            }
            BinaryOperationType::RightShift => {
                ir.current_register += 1;
                ir.statements.push(IRStatement::RightShift {
                    rd: Register(ir.current_register),
                    rs1: Register(left),
                    rs2: Register(right),
                });
                Ok(ir.current_register)
            }
            BinaryOperationType::Multiply => unimplemented!(),
            BinaryOperationType::Divide => unimplemented!(),
            BinaryOperationType::Modulus => unimplemented!(),
            BinaryOperationType::Exponentiate => unimplemented!(),
            BinaryOperationType::Equal => unimplemented!(),
            BinaryOperationType::NotEqual => unimplemented!(),
            BinaryOperationType::GreaterEqual => unimplemented!(),
            BinaryOperationType::LessEqual => unimplemented!(),
            BinaryOperationType::Greater => unimplemented!(),
            BinaryOperationType::Less => unimplemented!(),
            BinaryOperationType::LogicalAnd => unimplemented!(),
            BinaryOperationType::LogicalOr => unimplemented!(),
        }
    }
}
