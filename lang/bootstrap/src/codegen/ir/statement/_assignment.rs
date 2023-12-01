use crate::{
    ast::statement::AssignmentStatement,
    codegen::ir::{get_identifier_register, IRState, IRStatement, IRWalkable, Register},
    syntax_error::SyntaxError,
};

impl IRWalkable for AssignmentStatement {
    type Output = ();

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        let rd = match get_identifier_register(ir.scope.clone(), &self.identifier) {
            None => return Err(SyntaxError::AssignedUndeclaredVariable),
            Some(symbol) => symbol,
        };

        let rs1 = self.expression.walk_ir(ir)?;

        ir.current_register += 1;

        ir.statements.push(IRStatement::LoadImmediate {
            rd: Register(ir.current_register),
            imm: 0,
        });

        ir.statements.push(IRStatement::Add {
            rd: Register(rd),
            rs1: Register(rs1),
            rs2: Register(ir.current_register),
        });

        Ok(())
    }
}
