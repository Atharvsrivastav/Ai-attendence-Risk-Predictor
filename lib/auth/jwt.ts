import jwt from "jsonwebtoken";

const JWT_SECRET = process.env.JWT_SECRET || "attendance-risk-prediction-jwt-secret-key-987654321";

export interface TokenPayload {
  userId: string;
  email: string;
  name: string;
  role: "ADMIN" | "FACULTY" | "STUDENT";
  studentId?: string;
  facultyId?: string;
}

export function signToken(payload: TokenPayload): string {
  return jwt.sign(payload, JWT_SECRET, { expiresIn: "7d" });
}

export function verifyToken(token: string): TokenPayload | null {
  try {
    return jwt.verify(token, JWT_SECRET) as TokenPayload;
  } catch (error) {
    return null;
  }
}
