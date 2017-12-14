SELECT C.customerNumber, C.customerName FROM Customers C WHERE C.customerNumber NOT IN (SELECT O.customerNumber FROM Orders O);
